#include <cstring>

#include "microphone.hpp"

Microphone::Microphone(AudEngine *engine) :
        freeQueue_(nullptr), devShadowQueue_(nullptr) {
    SLresult result;

    // configure audio source
    SLDataLocator_IODevice loc_dev = {SL_DATALOCATOR_IODEVICE,
                                      SL_IODEVICE_AUDIOINPUT,
                                      SL_DEFAULTDEVICEID_AUDIOINPUT, nullptr};
    SLDataSource audioSrc = {&loc_dev, nullptr};

    // configure audio sink
    SLDataLocator_AndroidSimpleBufferQueue loc_bq = {
            SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, DEVICE_SHADOW_BUFFER_QUEUE_LEN};
    SLDataSink audioSnk = {&loc_bq, &pcmFormat};

    // create audio recorder (requires the RECORD_AUDIO permission)
    const SLInterfaceID id[2] = {SL_IID_ANDROIDSIMPLEBUFFERQUEUE, SL_IID_ANDROIDCONFIGURATION};
    const SLboolean req[2] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};
    result = (*engine->interface)->CreateAudioRecorder(
            engine->interface, &recObjectItf_, &audioSrc, &audioSnk, 1, id, req);
    SLASSERT(result);

    // Configure the voice recognition preset which has no signal processing for lower latency.
    SLAndroidConfigurationItf inputConfig;
    result = (*recObjectItf_)->GetInterface(
            recObjectItf_, SL_IID_ANDROIDCONFIGURATION, &inputConfig);
    if (SL_RESULT_SUCCESS == result) {
        SLuint32 presetValue = SL_ANDROID_RECORDING_PRESET_VOICE_RECOGNITION;
        (*inputConfig)->SetConfiguration(
                inputConfig, SL_ANDROID_KEY_RECORDING_PRESET, &presetValue, sizeof(SLuint32));
    }
    result = (*recObjectItf_)->Realize(recObjectItf_, SL_BOOLEAN_FALSE);
    SLASSERT(result);
    result = (*recObjectItf_)->GetInterface(recObjectItf_, SL_IID_RECORD, &recItf_);
    SLASSERT(result);

    result = (*recObjectItf_)->GetInterface(
            recObjectItf_, SL_IID_ANDROIDSIMPLEBUFFERQUEUE, &recBufQueueItf_);
    SLASSERT(result);

    result = (*recBufQueueItf_)->RegisterCallback(
            recBufQueueItf_,
            [](SLAndroidSimpleBufferQueueItf bq, void *recorder) {
                // Called for every buffer is full; pass directly to handler.
                (static_cast<Microphone *>(recorder))->ProcessSLCallback(bq, 0/*Queuer::Now()*/);
            }, this);
    SLASSERT(result);

    devShadowQueue_ = new ProducerConsumerQueue<sample_buf *>(DEVICE_SHADOW_BUFFER_QUEUE_LEN);
    assert(devShadowQueue_);

    silentBuf_.cap_ = (pcmFormat.containerSize >> 3) * pcmFormat.numChannels * FRAMES_PER_BUF;
    silentBuf_.buf_ = new uint8_t[silentBuf_.cap_];
    memset(silentBuf_.buf_, 0, silentBuf_.cap_);
    silentBuf_.size_ = silentBuf_.cap_;

    SetBufQueues();
}

/**
 * Compute the RECOMMENDED fast audio buffer size:
 * the lower latency required
 *   *) the smaller the buffer should be (adjust it here) AND
 *   *) the less buffering should be before starting player AFTER receiving the recorder buffer
 * Adjust the bufSize here to fit your bill [before it busts]
 */
void Microphone::SetBufQueues() {
    uint32_t bufSize = FRAMES_PER_BUF * pcmFormat.numChannels * pcmFormat.bitsPerSample;
    bufSize = (bufSize + 7) >> 3;  // bits --> byte
    bufCount_ = BUF_COUNT;
    bufs_ = allocateSampleBufs(bufCount_, bufSize);
    assert(bufs_);
    freeQueue_ = new ProducerConsumerQueue<sample_buf *>(bufCount_);
    for (uint32_t i = 0; i < bufCount_; i++)
        freeQueue_->push(&bufs_[i]);
}

bool Microphone::Start() {
    ASSERT(freeQueue_ && devShadowQueue_, "Some of the queues are NULL: (%p, %p)",
           freeQueue_, devShadowQueue_)
    audioBufCount = 0;

    SLresult result;
    // in case already recording, stop recording and clear buffer queue
    result = (*recItf_)->SetRecordState(recItf_, SL_RECORDSTATE_STOPPED);
    SLASSERT(result);
    result = (*recBufQueueItf_)->Clear(recBufQueueItf_);
    SLASSERT(result);

    for (int i = 0; i < RECORD_DEVICE_KICKSTART_BUF_COUNT; i++) {
        sample_buf *buf = nullptr;
        if (!freeQueue_->front(&buf)) {
            LOGE("=====OutOfFreeBuffers @ startingRecording @ (%d)", i);
            break;
        }
        freeQueue_->pop();
        assert(buf->buf_ && buf->cap_ && !buf->size_);

        result = (*recBufQueueItf_)->Enqueue(recBufQueueItf_, buf->buf_, buf->cap_);
        SLASSERT(result);
        devShadowQueue_->push(buf);
    }

    result = (*recItf_)->SetRecordState(recItf_, SL_RECORDSTATE_RECORDING);

#if AUD_SAVE
    std::string path = cacheDir + "aud.pcm";
    remove(path.c_str());
    test = std::ofstream(path, std::ios::binary);
#endif

    return result == SL_RESULT_SUCCESS;
}

void Microphone::ProcessSLCallback(SLAndroidSimpleBufferQueueItf bq, int64_t) {
    assert(bq == recBufQueueItf_);
    sample_buf *buf = nullptr;
    devShadowQueue_->front(&buf);
    devShadowQueue_->pop();
    buf->size_ = buf->cap_;  // device only calls us when it is really full

    sample_buf *freeBuf;
    while (freeQueue_->front(&freeBuf) && devShadowQueue_->push(freeBuf)) {
        freeQueue_->pop();
        SLresult result = (*bq)->Enqueue(bq, freeBuf->buf_, freeBuf->cap_);
        SLASSERT(result);
    }

    ++audioBufCount;

    // should leave the device to sleep to save power if no buffers
    assert(devShadowQueue_->size() != 0);
    // (*recItf_)->SetRecordState(recItf_, SL_RECORDSTATE_STOPPED);

    devShadowQueue_->pop();

    if (buf != &silentBuf_) {
#if AUD_SAVE
        //LOGE("%s", ("AUD: " + std::to_string(buf->size_)).c_str());
        test.write((char *) buf->buf_, buf->size_); // size=384 (FRAMES_PER_BUF*2)
#endif

        buf->size_ = 0;
        freeQueue_->push(buf);

        devShadowQueue_->push(buf);
        //(*bq)->Enqueue(bq, buf->buf_, buf->size_);
        // Caused  W  Leaving BufferQueue::Enqueue (SL_RESULT_PARAMETER_INVALID)
        return;
    }

    /*if (recQueue_->size() < PLAY_KICKSTART_BUFFER_COUNT) {
        (*bq)->Enqueue(bq, buf->buf_, buf->size_);
        devShadowQueue_->push(&silentBuf_);
        return;
    }*/

    assert(PLAY_KICKSTART_BUFFER_COUNT <=
           (DEVICE_SHADOW_BUFFER_QUEUE_LEN - devShadowQueue_->size()));
    for (int32_t idx = 0; idx < PLAY_KICKSTART_BUFFER_COUNT; idx++) {
        devShadowQueue_->push(buf);
        (*bq)->Enqueue(bq, buf->buf_, buf->size_);
    }
}

bool Microphone::Stop() {
#if AUD_SAVE
    test.close();
#endif

    // In case already recording, stop recording and clear buffer queue.
    SLuint32 curState;
    SLresult result = (*recItf_)->GetRecordState(recItf_, &curState);
    SLASSERT(result);
    if (curState == SL_RECORDSTATE_STOPPED) return true;
    assert(curState != SL_RECORDSTATE_STOPPED);
    result = (*recItf_)->SetRecordState(recItf_, SL_RECORDSTATE_STOPPED);
    SLASSERT(result);

    result = (*recBufQueueItf_)->Clear(recBufQueueItf_);
    SLASSERT(result);
    return true;
}

Microphone::~Microphone() {
    // destroy audio recorder object, and invalidate all associated interfaces
    if (recObjectItf_ != nullptr)
        (*recObjectItf_)->Destroy(recObjectItf_);

    // Consume all non-completed audio buffers
    sample_buf *buf = nullptr;
    if (devShadowQueue_) {
        while (devShadowQueue_->front(&buf)) {
            buf->size_ = 0;
            devShadowQueue_->pop();
            if (buf != &silentBuf_) freeQueue_->push(buf);
        }
        delete (devShadowQueue_);
    }
    delete freeQueue_;
    releaseSampleBufs(bufs_, bufCount_);
}

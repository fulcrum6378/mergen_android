#include "recorder.h"

// Called for every buffer is full; pass directly to handler.
void bqRecorderCallback(SLAndroidSimpleBufferQueueItf bq, void *recorder) {
    (static_cast<AudioRecorder *>(recorder))->ProcessSLCallback(bq);
}

void AudioRecorder::ProcessSLCallback(SLAndroidSimpleBufferQueueItf bq) {
    assert(bq == recBufQueueItf_);
    sample_buf *buf = nullptr;
    devShadowQueue_->front(&buf);
    devShadowQueue_->pop();
    buf->size_ = buf->cap_;  // device only calls us when it is really full
    recQueue_->push(buf);

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
        test.write((char *) buf->buf_, buf->size_);

        buf->size_ = 0;
        freeQueue_->push(buf);

        if (!recQueue_->front(&buf)) return;

        devShadowQueue_->push(buf);
        //(*bq)->Enqueue(bq, buf->buf_, buf->size_);
        // Caused  W  Leaving BufferQueue::Enqueue (SL_RESULT_PARAMETER_INVALID)
        recQueue_->pop();
        return;
    }

    if (recQueue_->size() < PLAY_KICKSTART_BUFFER_COUNT) {
        (*bq)->Enqueue(bq, buf->buf_, buf->size_);
        devShadowQueue_->push(&silentBuf_);
        return;
    }

    assert(PLAY_KICKSTART_BUFFER_COUNT <=
           (DEVICE_SHADOW_BUFFER_QUEUE_LEN - devShadowQueue_->size()));
    for (int32_t idx = 0; idx < PLAY_KICKSTART_BUFFER_COUNT; idx++) {
        recQueue_->front(&buf);
        recQueue_->pop();
        devShadowQueue_->push(buf);
        (*bq)->Enqueue(bq, buf->buf_, buf->size_);
    }
}

AudioRecorder::AudioRecorder(SampleFormat *sampleFormat, SLEngineItf slEngine)
        : freeQueue_(nullptr), recQueue_(nullptr), devShadowQueue_(nullptr) {
    SLresult result;
    sampleInfo_ = *sampleFormat;
    SLAndroidDataFormat_PCM_EX format_pcm;
    ConvertToSLSampleFormat(&format_pcm, &sampleInfo_);

    // configure audio source
    SLDataLocator_IODevice loc_dev = {SL_DATALOCATOR_IODEVICE,
                                      SL_IODEVICE_AUDIOINPUT,
                                      SL_DEFAULTDEVICEID_AUDIOINPUT, nullptr};
    SLDataSource audioSrc = {&loc_dev, nullptr};

    // configure audio sink
    SLDataLocator_AndroidSimpleBufferQueue loc_bq = {
            SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, DEVICE_SHADOW_BUFFER_QUEUE_LEN};

    SLDataSink audioSnk = {&loc_bq, &format_pcm};

    // create audio recorder (requires the RECORD_AUDIO permission)
    const SLInterfaceID id[2] = {SL_IID_ANDROIDSIMPLEBUFFERQUEUE, SL_IID_ANDROIDCONFIGURATION};
    const SLboolean req[2] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};
    result = (*slEngine)->CreateAudioRecorder(
            slEngine, &recObjectItf_, &audioSrc, &audioSnk, 1, id, req);
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

    result = (*recObjectItf_)->GetInterface(recObjectItf_, SL_IID_ANDROIDSIMPLEBUFFERQUEUE,
                                            &recBufQueueItf_);
    SLASSERT(result);

    result = (*recBufQueueItf_)->RegisterCallback(recBufQueueItf_, bqRecorderCallback, this);
    SLASSERT(result);

    devShadowQueue_ = new AudioQueue(DEVICE_SHADOW_BUFFER_QUEUE_LEN);
    assert(devShadowQueue_);

    silentBuf_.cap_ = (format_pcm.containerSize >> 3) * format_pcm.numChannels *
                      sampleInfo_.framesPerBuf_;
    silentBuf_.buf_ = new uint8_t[silentBuf_.cap_];
    memset(silentBuf_.buf_, 0, silentBuf_.cap_);
    silentBuf_.size_ = silentBuf_.cap_;
}

SLboolean AudioRecorder::Start() {
    if (!freeQueue_ || !recQueue_ || !devShadowQueue_) {
        LOGE("====NULL poiter to Start(%p, %p, %p)", freeQueue_, recQueue_, devShadowQueue_);
        return SL_BOOLEAN_FALSE;
    }
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

    char path[] = "/data/data/ir.mahdiparastesh.mergen/files/aud.pcm";
    remove(path);
    test = std::ofstream(path, std::ios::binary | std::ios::out);

    return (result == SL_RESULT_SUCCESS ? SL_BOOLEAN_TRUE : SL_BOOLEAN_FALSE);
}

SLboolean AudioRecorder::Stop() {
    test.close();

    // In case already recording, stop recording and clear buffer queue.
    SLuint32 curState;
    SLresult result = (*recItf_)->GetRecordState(recItf_, &curState);
    SLASSERT(result);
    if (curState == SL_RECORDSTATE_STOPPED) return SL_BOOLEAN_TRUE;
    assert(curState != SL_RECORDSTATE_STOPPED);
    result = (*recItf_)->SetRecordState(recItf_, SL_RECORDSTATE_STOPPED);
    SLASSERT(result);

    result = (*recBufQueueItf_)->Clear(recBufQueueItf_);
    SLASSERT(result);
    return SL_BOOLEAN_TRUE;
}

AudioRecorder::~AudioRecorder() {
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
        while (recQueue_->front(&buf)) {
            buf->size_ = 0;
            recQueue_->pop();
            freeQueue_->push(buf);
        }
    }
}

void AudioRecorder::SetBufQueues(AudioQueue *freeQ, AudioQueue *recQ) {
    assert(freeQ && recQ);
    freeQueue_ = freeQ;
    recQueue_ = recQ;
}

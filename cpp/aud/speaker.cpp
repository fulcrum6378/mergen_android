#include <cstring>

#include "speaker.hpp"

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"

/*
 * Called by OpenSL SimpleBufferQueue for every audio buffer played
 * directly pass through to our handler.
 * The regularity of this callback from openSL/Android System affects
 * playback continuity. If it does not call back in the regular time
 * slot, you are under big pressure for audio processing[here we do
 * not do any filtering/mixing]. Callback from fast audio path are
 * much more regular than other audio paths by my observation. If it
 * is very regular, you could buffer much less audio samples between
 * recorder and player, hence lower latency.
 */
void bqPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *ctx) {
    (static_cast<Speaker *>(ctx))->ProcessSLCallback(bq);
}

void Speaker::ProcessSLCallback(SLAndroidSimpleBufferQueueItf bq) {
    std::lock_guard <std::mutex> lock(stopMutex_);

    // retrieve the finished device buf and put onto the free queue so recorder could re-use it
    sample_buf *buf;
    if (!devShadowQueue_->front(&buf))return;
    devShadowQueue_->pop();

    if (buf != &silentBuf_) {
        buf->size_ = 0;
        freeQueue_->push(buf);

        if (!playQueue_->front(&buf)) return;

        devShadowQueue_->push(buf);
        (*bq)->Enqueue(bq, buf->buf_, buf->size_);
        playQueue_->pop();
        return;
    }

    if (playQueue_->size() < PLAY_KICKSTART_BUFFER_COUNT) {
        (*bq)->Enqueue(bq, buf->buf_, buf->size_);
        devShadowQueue_->push(&silentBuf_);
        return;
    }

    assert(PLAY_KICKSTART_BUFFER_COUNT <=
           (DEVICE_SHADOW_BUFFER_QUEUE_LEN - devShadowQueue_->size()));
    for (int32_t idx = 0; idx < PLAY_KICKSTART_BUFFER_COUNT; idx++) {
        playQueue_->front(&buf);
        playQueue_->pop();
        devShadowQueue_->push(buf);
        (*bq)->Enqueue(bq, buf->buf_, buf->size_);
    }
}

Speaker::Speaker(SampleFormat *sampleFormat, SLEngineItf slEngine)
        : freeQueue_(nullptr), playQueue_(nullptr), devShadowQueue_(nullptr) {
    SLresult result;
    assert(sampleFormat);
    sampleInfo_ = *sampleFormat;

    result = (*slEngine)->CreateOutputMix(
            slEngine, &outputMixObjectItf_, 0, nullptr, nullptr);
    SLASSERT(result);

    // realize the output mix
    result = (*outputMixObjectItf_)->Realize(outputMixObjectItf_, SL_BOOLEAN_FALSE);
    SLASSERT(result);

    // configure audio source
    SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {
            SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, DEVICE_SHADOW_BUFFER_QUEUE_LEN};

    SLAndroidDataFormat_PCM_EX format_pcm;
    ConvertToSLSampleFormat(&format_pcm, &sampleInfo_);
    SLDataSource audioSrc = {&loc_bufq, &format_pcm};

    // configure audio sink
    SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX,
                                          outputMixObjectItf_};
    SLDataSink audioSnk = {&loc_outmix, nullptr};
    /*
     * create fast path audio player: SL_IID_BUFFERQUEUE and SL_IID_VOLUME
     * and other non-signal processing interfaces are ok.
     */
    SLInterfaceID ids[2] = {SL_IID_BUFFERQUEUE, SL_IID_VOLUME};
    SLboolean req[2] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};
    result = (*slEngine)->CreateAudioPlayer(
            slEngine, &playerObjectItf_, &audioSrc, &audioSnk,
            sizeof(ids) / sizeof(ids[0]), ids, req);
    SLASSERT(result);

    // realize the player
    result = (*playerObjectItf_)->Realize(playerObjectItf_, SL_BOOLEAN_FALSE);
    SLASSERT(result);

    // get the play interface
    result = (*playerObjectItf_)
            ->GetInterface(playerObjectItf_, SL_IID_PLAY, &playItf_);
    SLASSERT(result);

    // get the buffer queue interface
    result = (*playerObjectItf_)
            ->GetInterface(playerObjectItf_, SL_IID_BUFFERQUEUE,
                           &playBufferQueueItf_);
    SLASSERT(result);

    // register callback on the buffer queue
    result = (*playBufferQueueItf_)
            ->RegisterCallback(playBufferQueueItf_, bqPlayerCallback, this);
    SLASSERT(result);

    result = (*playItf_)->SetPlayState(playItf_, SL_PLAYSTATE_STOPPED);
    SLASSERT(result);

    // create an empty queue to track deviceQueue
    devShadowQueue_ = new ProducerConsumerQueue<sample_buf *>(DEVICE_SHADOW_BUFFER_QUEUE_LEN);
    assert(devShadowQueue_);

    silentBuf_.cap_ = (format_pcm.containerSize >> 3) * format_pcm.numChannels *
                      sampleInfo_.framesPerBuf_;
    silentBuf_.buf_ = new uint8_t[silentBuf_.cap_];
    memset(silentBuf_.buf_, 0, silentBuf_.cap_);
    silentBuf_.size_ = silentBuf_.cap_;
}

Speaker::~Speaker() {
    std::lock_guard <std::mutex> lock(stopMutex_);

    // destroy buffer queue audio player object, and invalidate all associated
    // interfaces
    if (playerObjectItf_ != nullptr)
        (*playerObjectItf_)->Destroy(playerObjectItf_);

    // Consume all non-completed audio buffers
    sample_buf *buf = nullptr;
    while (devShadowQueue_->front(&buf)) {
        buf->size_ = 0;
        devShadowQueue_->pop();
        if (buf != &silentBuf_) freeQueue_->push(buf);
    }
    delete devShadowQueue_;

    while (playQueue_->front(&buf)) {
        buf->size_ = 0;
        playQueue_->pop();
        freeQueue_->push(buf);
    }

    // destroy output mix object, and invalidate all associated interfaces
    if (outputMixObjectItf_) (*outputMixObjectItf_)->Destroy(outputMixObjectItf_);

    delete[] silentBuf_.buf_;
}

void Speaker::SetBufQueue(
        ProducerConsumerQueue<sample_buf *> *playQ, ProducerConsumerQueue<sample_buf *> *freeQ) {
    playQueue_ = playQ;
    freeQueue_ = freeQ;
}

SLresult Speaker::Start() {
    SLuint32 state;
    SLresult result = (*playItf_)->GetPlayState(playItf_, &state);
    if (result != SL_RESULT_SUCCESS) return SL_BOOLEAN_FALSE;
    if (state == SL_PLAYSTATE_PLAYING) return SL_BOOLEAN_TRUE;

    result = (*playItf_)->SetPlayState(playItf_, SL_PLAYSTATE_STOPPED);
    SLASSERT(result);

    result = (*playBufferQueueItf_)
            ->Enqueue(playBufferQueueItf_, silentBuf_.buf_, silentBuf_.size_);
    SLASSERT(result);
    devShadowQueue_->push(&silentBuf_);

    result = (*playItf_)->SetPlayState(playItf_, SL_PLAYSTATE_PLAYING);
    SLASSERT(result);
    return SL_BOOLEAN_TRUE;
}

void Speaker::Stop() {
    SLuint32 state;

    SLresult result = (*playItf_)->GetPlayState(playItf_, &state);
    SLASSERT(result);

    if (state == SL_PLAYSTATE_STOPPED) return;

    std::lock_guard <std::mutex> lock(stopMutex_);

    result = (*playItf_)->SetPlayState(playItf_, SL_PLAYSTATE_STOPPED);
    SLASSERT(result);
    (*playBufferQueueItf_)->Clear(playBufferQueueItf_);
}

#pragma clang diagnostic pop

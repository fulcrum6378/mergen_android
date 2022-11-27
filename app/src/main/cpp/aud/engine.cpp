#include "engine.h"

AudioEngine::AudioEngine(int32_t sampleRate, int32_t framesPerBuf) :
        slEngineObj_(nullptr), slEngineItf_(nullptr), recorder_(nullptr) {
    fastPathSampleRate_ = static_cast<SLmilliHertz>(sampleRate) * 1000;
    fastPathFramesPerBuf_ = static_cast<uint32_t>(framesPerBuf);
    sampleChannels_ = AUDIO_SAMPLE_CHANNELS;
    bitsPerSample_ = SL_PCMSAMPLEFORMAT_FIXED_16;

    SLresult result;
    result = slCreateEngine(
            &slEngineObj_, 0, nullptr, 0,
            nullptr, nullptr);
    SLASSERT(result);

    result = (*slEngineObj_)->Realize(slEngineObj_, SL_BOOLEAN_FALSE);
    SLASSERT(result);

    result = (*slEngineObj_)->GetInterface(slEngineObj_, SL_IID_ENGINE, &slEngineItf_);
    SLASSERT(result);

    // compute the RECOMMENDED fast audio buffer size:
    //   the lower latency required
    //     *) the smaller the buffer should be (adjust it here) AND
    //     *) the less buffering should be before starting player AFTER
    //        receiving the recorder buffer
    //   Adjust the bufSize here to fit your bill [before it busts]
    uint32_t bufSize = fastPathFramesPerBuf_ * sampleChannels_ * bitsPerSample_;
    bufSize = (bufSize + 7) >> 3;  // bits --> byte
    bufCount_ = BUF_COUNT;
    bufs_ = allocateSampleBufs(bufCount_, bufSize);
    assert(bufs_);
    freeBufQueue_ = new AudioQueue(bufCount_);
    recBufQueue_ = new AudioQueue(bufCount_);
    assert(freeBufQueue_ && recBufQueue_);
    for (uint32_t i = 0; i < bufCount_; i++)
        freeBufQueue_->push(&bufs_[i]);
}

AudioEngine::~AudioEngine() {
    delete recorder_;
    recorder_ = nullptr;
    delete recBufQueue_;
    delete freeBufQueue_;
    releaseSampleBufs(bufs_, bufCount_);
    if (slEngineObj_ != nullptr) {
        (*slEngineObj_)->Destroy(slEngineObj_);
        slEngineObj_ = nullptr;
        slEngineItf_ = nullptr;
    }
}

bool AudioEngine::StartRecording() {
    SampleFormat sampleFormat{
            fastPathSampleRate_,
            fastPathFramesPerBuf_,
            sampleChannels_,
            static_cast<uint16_t>(bitsPerSample_),
            0
    };
    recorder_ = new AudioRecorder(&sampleFormat, slEngineItf_);
    recorder_->SetBufQueues(freeBufQueue_, recBufQueue_);
    return recorder_->Start();
}

bool AudioEngine::StopRecording() {
    if (!recorder_->Stop()) return false;
    delete recorder_;
    recorder_ = nullptr;
    return true;
}

#include "engine.h"

AudioEngine::AudioEngine(Queuer *queuer) :
        slEngineObj_(nullptr), slEngineItf_(nullptr), recorder_(nullptr), queuer_(queuer) {
    SLresult result;
    result = slCreateEngine(
            &slEngineObj_, 0, nullptr, 0,
            nullptr, nullptr);
    SLASSERT(result);

    result = (*slEngineObj_)->Realize(slEngineObj_, SL_BOOLEAN_FALSE);
    SLASSERT(result);

    result = (*slEngineObj_)->GetInterface(slEngineObj_, SL_IID_ENGINE, &slEngineItf_);
    SLASSERT(result);
}

bool AudioEngine::StartRecording() {
    recorder_ = new AudioRecorder(slEngineItf_, queuer_);
    return recorder_->Start();
}

bool AudioEngine::StopRecording() {
    if (!recorder_->Stop()) return false;
    delete recorder_;
    recorder_ = nullptr;
    return true;
}

AudioEngine::~AudioEngine() {
    delete recorder_;
    recorder_ = nullptr;
    if (slEngineObj_ != nullptr) {
        (*slEngineObj_)->Destroy(slEngineObj_);
        slEngineObj_ = nullptr;
        slEngineItf_ = nullptr;
    }
}

#include "speaker.hpp"

Speaker::Speaker() {
    AAudioStreamBuilder *builder;
    AAudio_createStreamBuilder(&builder);
    AAudioStreamBuilder_setDeviceId(builder, AAUDIO_UNSPECIFIED);
    AAudioStreamBuilder_setDirection(builder, AAUDIO_DIRECTION_OUTPUT);
    AAudioStreamBuilder_setSharingMode(builder, AAUDIO_SHARING_MODE_SHARED);
    AAudioStreamBuilder_setSampleRate(builder, 48000);
    AAudioStreamBuilder_setChannelCount(builder, 1);
    AAudioStreamBuilder_setFormat(builder, AAUDIO_FORMAT_PCM_I16);
    AAudioStreamBuilder_setBufferCapacityInFrames(builder, 192);
    AAudioStreamBuilder_setDataCallback(builder, Speaker::Callback, this);
    AAudioStreamBuilder_openStream(builder, &stream);
    AAudioStreamBuilder_delete(builder);
}

bool Speaker::IsStarted() const {
    return state == AAUDIO_STREAM_STATE_STARTED || state == AAUDIO_STREAM_STATE_STARTING;
}

bool Speaker::Start(void (*callback)(int16_t *, int32_t, void *), void *callbackData) {
    if (callback_ != nullptr) return false;
    aaudio_result_t result = AAudioStream_requestStart(stream);
    if (result != AAUDIO_OK) return false;
    result = AAudioStream_waitForStateChange(
            stream, AAUDIO_STREAM_STATE_STARTING, &state, waitTimeout);
    if (result != AAUDIO_OK) return false;
    else {
        callback_ = callback;
        callbackData_ = callbackData;
    }
    return true;
}

aaudio_data_callback_result_t Speaker::Callback(
        AAudioStream */*stream*/, void *data, void *buf, int32_t numFrames) {
    auto *speaker = static_cast<Speaker *>(data);
    if (speaker->callback_ == nullptr)
        return AAUDIO_CALLBACK_RESULT_CONTINUE; // don't use `..._CONTINUE`
    speaker->callback_(static_cast<int16_t *>(buf), numFrames, speaker->callbackData_);
    return AAUDIO_CALLBACK_RESULT_CONTINUE;
}

void Speaker::Stop() {
    callback_ = nullptr;
    callbackData_ = nullptr;
    AAudioStream_requestStop(stream);
    AAudioStream_waitForStateChange(
            stream, AAUDIO_STREAM_STATE_STOPPING, &state, waitTimeout);
}

Speaker::~Speaker() {
    AAudioStream_close(stream);
}

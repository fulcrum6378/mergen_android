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

void Speaker::Start() {
    aaudio_result_t result = AAudioStream_requestStart(stream);
    //if (result != AAUDIO_OK) return false;
    result = AAudioStream_waitForStateChange(
            stream, AAUDIO_STREAM_STATE_STARTING, &state, waitTimeout);
    //if (result != AAUDIO_OK) return false;
    //return true;
}

aaudio_data_callback_result_t Speaker::Callback(
        AAudioStream */*stream*/, void *microphone, void *buf, int32_t numFrames) {
    auto *aud_out = static_cast<Speaker *>(microphone);
    // control `static_cast<int16_t *>(buf), numFrames` to make noises
    return AAUDIO_CALLBACK_RESULT_CONTINUE;
}

void Speaker::Stop() {
    /*result = AAudioStream_requestPause(stream);
    LOGW("%d", result);
    if (result != AAUDIO_OK) return false;
    result = AAudioStream_waitForStateChange(
            stream, AAUDIO_STREAM_STATE_PAUSING, &state, waitTimeout);
    LOGW("%d", result);
    if (result != AAUDIO_OK) return false;
    result = AAudioStream_requestFlush(stream);
    LOGW("%d", result);
    if (result != AAUDIO_OK) return false;
    result = AAudioStream_waitForStateChange(
            stream, AAUDIO_STREAM_STATE_FLUSHING, &state, waitTimeout);
    LOGW("%d", result);
    if (result != AAUDIO_OK) return false;
    return true;*/
}

Speaker::~Speaker() {
    AAudioStream_requestStop(stream);
    AAudioStream_waitForStateChange(
            stream, AAUDIO_STREAM_STATE_STOPPING, &state, waitTimeout);
    AAudioStream_close(stream);
}

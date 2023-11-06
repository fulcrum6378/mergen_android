#include "speaker.hpp"

Speaker::Speaker() {}

void Speaker::Start() {}

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

Speaker::~Speaker() {}

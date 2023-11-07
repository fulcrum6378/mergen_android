#include <cstdlib>

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

void Speaker::Start() {
    AAudioStream_requestStart(stream);
    AAudioStream_waitForStateChange(
            stream, AAUDIO_STREAM_STATE_STARTING, &state, waitTimeout);
}

aaudio_data_callback_result_t Speaker::Callback(
        AAudioStream */*stream*/, void */*microphone*/, void *buf, int32_t numFrames) {
    auto *buf_ = static_cast<int16_t *>(buf);

    int togval = 0, togmax = 20, overval = 0, overmax = 400;
    float x, overadd = 0.1;
    bool togg = true, over = false;
    for (int f = 0; f < numFrames; f++) {

        x = 0;
        int median = numFrames / 2;
        if (f < median)
            x += (float) f;
        else
            x += (float) abs(f - numFrames);
        //x *= 0.000001;

        if (over) x += (float) (overadd * (x + 0.1));
        if (overval >= overmax) {
            over = !over;
            overval = 0;
        } else overval += 1;

        if (!togg) x = 0 - x;
        if (togval >= togmax) {
            togg = !togg;
            togval = 0;
        } else togval += 1;

        buf_[f] = static_cast<int16_t>(x * 32767); //(rand() % 65535) - 32768;
    }
    //auto *aud_out = static_cast<Speaker *>(microphone);
    return AAUDIO_CALLBACK_RESULT_CONTINUE;
}

void Speaker::Stop() {
    AAudioStream_requestStop(stream);
    AAudioStream_waitForStateChange(
            stream, AAUDIO_STREAM_STATE_STOPPING, &state, waitTimeout);
}

Speaker::~Speaker() {
    AAudioStream_requestStop(stream);
    AAudioStream_waitForStateChange(
            stream, AAUDIO_STREAM_STATE_STOPPING, &state, waitTimeout);
    AAudioStream_close(stream);
}

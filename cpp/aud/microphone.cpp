#include "../global.hpp"
#include "microphone.hpp"

Microphone::Microphone() {
    AAudioStreamBuilder *builder;
    AAudio_createStreamBuilder(&builder);
    AAudioStreamBuilder_setDeviceId(builder, AAUDIO_UNSPECIFIED);
    AAudioStreamBuilder_setDirection(builder, AAUDIO_DIRECTION_INPUT);
    AAudioStreamBuilder_setSharingMode(builder, AAUDIO_SHARING_MODE_SHARED);
    AAudioStreamBuilder_setSampleRate(builder, 48000);
    AAudioStreamBuilder_setChannelCount(builder, 1);
    AAudioStreamBuilder_setFormat(builder, AAUDIO_FORMAT_PCM_I16);
    AAudioStreamBuilder_setBufferCapacityInFrames(builder, 192);
    //AAudioStreamBuilder_setPerformanceMode(builder, AAUDIO_PERFORMANCE_MODE_NONE);
    // or AAUDIO_PERFORMANCE_MODE_LOW_LATENCY or AAUDIO_PERFORMANCE_MODE_POWER_SAVING
    AAudioStreamBuilder_setDataCallback(builder, Microphone::Callback, nullptr);
    AAudioStreamBuilder_openStream(builder, &stream);
    AAudioStreamBuilder_delete(builder);
}

bool Microphone::Start() {
    aaudio_result_t result = AAudioStream_requestStart(stream);
    if (result != AAUDIO_OK) return false;
    result = AAudioStream_waitForStateChange(
            stream, AAUDIO_STREAM_STATE_STARTING, &state, waitTimeout);
    if (result != AAUDIO_OK) return false;
#if AUD_IN_SAVE
    testPCM = std::ofstream((cacheDir + "aud.pcm").c_str(), std::ios::binary);
#endif
    return true;
}

aaudio_data_callback_result_t Microphone::Callback(
        AAudioStream */*stream*/, void */*userData*/, void *audioData, int32_t numFrames) {
    // Write samples directly into the audioData array.
    // generateSineWave(static_cast<int16_t *>(audioData), numFrames);
#if AUD_IN_SAVE
    testPCM.write((char *) audioData, numFrames * 2);
#endif
    return AAUDIO_CALLBACK_RESULT_CONTINUE;
}

bool Microphone::Stop() {
    aaudio_result_t result = AAudioStream_requestStop(stream);
    if (result != AAUDIO_OK) return false;
    result = AAudioStream_waitForStateChange(
            stream, AAUDIO_STREAM_STATE_STOPPING, &state, waitTimeout);
#if AUD_IN_SAVE
    testPCM.close();
#endif
    return result == AAUDIO_OK; // better close `testPCM` and then return
}

Microphone::~Microphone() {
    AAudioStream_requestStop(stream);
    AAudioStream_waitForStateChange(
            stream, AAUDIO_STREAM_STATE_STOPPING, &state, waitTimeout);
    AAudioStream_close(stream);
}

#include "../global.hpp"
#include "microphone.hpp"

Microphone::Microphone() {
    AAudioStreamBuilder *builder;
    AAudio_createStreamBuilder(&builder);
    AAudioStreamBuilder_setDeviceId(builder, AAUDIO_UNSPECIFIED);
    AAudioStreamBuilder_setDirection(builder, AAUDIO_DIRECTION_INPUT);
    AAudioStreamBuilder_setSharingMode(builder, AAUDIO_SHARING_MODE_SHARED);
    AAudioStreamBuilder_setSampleRate(builder, 44100); // 2 sample hertz reconstruct a hearing wavelength!
    AAudioStreamBuilder_setChannelCount(builder, 1);
    AAudioStreamBuilder_setFormat(builder, AAUDIO_FORMAT_PCM_I16);
    // TODO we better make them 8-bit: AAUDIO_FORMAT_PCM_I24_PACKED. Implementation below should be fixed.
    AAudioStreamBuilder_setBufferCapacityInFrames(builder, 192);
    AAudioStreamBuilder_setDataCallback(builder, Microphone::Callback, this);
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
        AAudioStream */*stream*/, void *microphone, void *buf, int32_t numFrames) {
    auto *aud_in = static_cast<Microphone *>(microphone);
#if AUD_IN_SAVE
    aud_in->testPCM.write((char *) buf, numFrames * 2);
#endif
    // pass `static_cast<int16_t *>(buf), numFrames` to the next step
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
    AAudioStream_close(stream);
}

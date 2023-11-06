#ifndef AUD_MICROPHONE_H
#define AUD_MICROPHONE_H

#include <aaudio/AAudio.h>
#include <fstream>

#define AUD_IN_SAVE true

inline std::ofstream testPCM;

class Microphone {
private:
    AAudioStream *stream{};

    aaudio_stream_state_t state{AAUDIO_STREAM_STATE_UNINITIALIZED};
    int64_t waitTimeout{100L * /*AAUDIO_NANOS_PER_MILLISECOND:*/1000000L};

    static aaudio_data_callback_result_t
    Callback(AAudioStream *stream, void *userData, void *audioData, int32_t numFrames);

public:
    Microphone();

    bool Start();

    bool Stop();

    ~Microphone();
};

#endif //AUD_MICROPHONE_H

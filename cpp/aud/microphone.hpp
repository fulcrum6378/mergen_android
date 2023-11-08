#ifndef AUD_MICROPHONE_H
#define AUD_MICROPHONE_H

#include <aaudio/AAudio.h>
#include <fstream>

#define AUD_IN_SAVE true

class Microphone {
public:
    Microphone();

    bool Start();

    bool Stop();

    ~Microphone();


    std::ofstream testPCM;

private:
    static aaudio_data_callback_result_t
    Callback(AAudioStream *stream, void *microphone, void *buf, int32_t numFrames);


    AAudioStream *stream{};
    aaudio_stream_state_t state{AAUDIO_STREAM_STATE_UNINITIALIZED};
    int64_t waitTimeout{300000000ll}; // nanoseconds (billionth), 300 milliseconds
};

#endif //AUD_MICROPHONE_H

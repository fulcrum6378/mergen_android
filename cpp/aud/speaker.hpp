#ifndef AUD_SPEAKER_H
#define AUD_SPEAKER_H

#include <aaudio/AAudio.h>

class Speaker {
private:
    AAudioStream *stream{};
    aaudio_stream_state_t state{AAUDIO_STREAM_STATE_UNINITIALIZED};
    int64_t waitTimeout{300000000L}; // nanoseconds (billionth), 300 milliseconds

    static aaudio_data_callback_result_t
    Callback(AAudioStream *stream, void *speaker, void *buf, int32_t numFrames);

public:
    Speaker();

    bool IsStarted() const;

    void Start();

    void Stop();

    ~Speaker();
};

#endif //AUD_SPEAKER_H

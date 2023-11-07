#ifndef AUD_SPEAKER_H
#define AUD_SPEAKER_H

#include <aaudio/AAudio.h>

class Speaker {
private:
    AAudioStream *stream{};
    aaudio_stream_state_t state{AAUDIO_STREAM_STATE_UNINITIALIZED};
    int64_t waitTimeout{300000000L}; // nanoseconds (billionth), 300 milliseconds

    void (*callback_)(int16_t *, int32_t, void *){nullptr};

    void *callbackData_ = nullptr;

    static aaudio_data_callback_result_t
    Callback(AAudioStream *stream, void *speaker, void *buf, int32_t numFrames);

public:
    Speaker();

    bool IsStarted() const;

    bool Start(void (*callback)(int16_t *, int32_t, void *), void *callbackData);

    void Stop();

    ~Speaker();
};

#endif //AUD_SPEAKER_H

#ifndef AUD_MICROPHONE_H
#define AUD_MICROPHONE_H

#include <fstream>

#include "commons.hpp"

#define AUD_SAVE true

class Microphone {
private:
    SLAndroidDataFormat_PCM_EX pcmFormat{
            SL_DATAFORMAT_PCM,
            1,
            SL_SAMPLINGRATE_48, // 48000000 millihertz
            SL_PCMSAMPLEFORMAT_FIXED_16, // namely "bit depth"
            SL_PCMSAMPLEFORMAT_FIXED_16, // change both together
            SL_SPEAKER_FRONT_LEFT,
            SL_BYTEORDER_LITTLEENDIAN, // FIXME
            SL_ANDROID_PCM_REPRESENTATION_SIGNED_INT, // TODO
            // SL_ANDROID_PCM_REPRESENTATION_UNSIGNED_INT, SL_ANDROID_PCM_REPRESENTATION_SIGNED_INT,
            // SL_ANDROID_PCM_REPRESENTATION_FLOAT
    };

    SLObjectItf recObjectItf_{};
    SLRecordItf recItf_{};
    SLAndroidSimpleBufferQueueItf recBufQueueItf_{};

    sample_buf *bufs_{};
    uint32_t bufCount_{};
    ProducerConsumerQueue<sample_buf *> *freeQueue_;       // user
    ProducerConsumerQueue<sample_buf *> *recQueue_;        // user
    // recBufQueue_ is recQueue_ for the recorder and playQueue_ for the player.
    ProducerConsumerQueue<sample_buf *> *devShadowQueue_;  // owner
    uint32_t audioBufCount{};
    sample_buf silentBuf_{};

    std::ofstream test;

    void SetBufQueues();

    void ProcessSLCallback(SLAndroidSimpleBufferQueueItf bq, int64_t time);

public:
    Microphone(AudEngine *engine);

    bool Start();

    bool Stop();

    ~Microphone();
};

#endif //AUD_MICROPHONE_H

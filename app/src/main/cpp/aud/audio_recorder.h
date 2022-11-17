#ifndef AUD_RECORDER_H
#define AUD_RECORDER_H

#include <fstream> // required for std::ofstream
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <sys/types.h>

#include "audio_common.h"
#include "buf_manager.h"

class AudioRecorder {
    SLObjectItf recObjectItf_{};
    SLRecordItf recItf_{};
    SLAndroidSimpleBufferQueueItf recBufQueueItf_{};

    SampleFormat sampleInfo_{};
    AudioQueue *freeQueue_;       // user
    AudioQueue *recQueue_;        // user
    AudioQueue *devShadowQueue_;  // owner
    uint32_t audioBufCount{};

    ENGINE_CALLBACK callback_;
    void *ctx_{};

public:
    explicit AudioRecorder(SampleFormat *, SLEngineItf engineEngine);

    std::ofstream myfile; // TODO

    ~AudioRecorder();

    SLboolean Start(void);

    SLboolean Stop(void);

    void SetBufQueues(AudioQueue *freeQ, AudioQueue *recQ);

    void ProcessSLCallback(SLAndroidSimpleBufferQueueItf bq);
};

#endif //AUD_RECORDER_H

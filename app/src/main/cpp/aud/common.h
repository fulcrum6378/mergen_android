#ifndef AUD_COMMON_H
#define AUD_COMMON_H

#include <SLES/OpenSLES_Android.h>

#include "../otr/debug.h" // do not include "recorder.h" here!

// Audio Sample Controls
#define SAMPLE_RATE 48000000 // millihertz
#define FRAMES_PER_BUF 192
#define AUDIO_SAMPLE_CHANNELS 1
#define BITS_PER_SAMPLE SL_PCMSAMPLEFORMAT_FIXED_16

// Sample Buffer Controls
#define RECORD_DEVICE_KICKSTART_BUF_COUNT 2
#define PLAY_KICKSTART_BUFFER_COUNT 3
#define DEVICE_SHADOW_BUFFER_QUEUE_LEN 4
#define BUF_COUNT 16

struct SampleFormat {
    uint32_t sampleRate_;
    uint32_t framesPerBuf_;
    uint16_t channels_;
    uint16_t pcmFormat_;  // 8 bit, 16 bit, 24 bit ...
    uint32_t representation_;  // android extensions
};

void ConvertToSLSampleFormat(SLAndroidDataFormat_PCM_EX *pFormat, SampleFormat *format);

#define SLASSERT(x)                   \
  do {                                \
    assert(SL_RESULT_SUCCESS == (x)); \
    (void)(x);                        \
  } while (0)

#endif //AUD_COMMON_H

#ifndef NATIVE_AUDIO_AUDIO_COMMON_H
#define NATIVE_AUDIO_AUDIO_COMMON_H

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

#include "../native_debug.h"
#include "debug_utils.h"
#include "buf_manager.h"

/*
 * Sample Buffer Controls...
 */
#define RECORD_DEVICE_KICKSTART_BUF_COUNT 2
#define DEVICE_SHADOW_BUFFER_QUEUE_LEN 4

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

/*
 * Interface for player and recorder to communicate with engine
 */
#define ENGINE_SERVICE_MSG_RECORDED_AUDIO_AVAILABLE 3

typedef bool (*ENGINE_CALLBACK)(void *pCTX, uint32_t msg, void *pData);

#endif

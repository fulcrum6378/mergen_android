#include <cassert>
#include <cstring>

#include "commons.h"

Engine::Engine() {
    SLresult result;
    result = slCreateEngine(
            &slEngineObj_, 0, nullptr, 0,
            nullptr, nullptr);
    SLASSERT(result);
    result = (*slEngineObj_)->Realize(slEngineObj_, SL_BOOLEAN_FALSE);
    SLASSERT(result);
    result = (*slEngineObj_)->GetInterface(slEngineObj_, SL_IID_ENGINE, &slEngineItf_);
    SLASSERT(result);
}

SLEngineItf Engine::GetSlEngineItf() {
    return slEngineItf_;
}

Engine::~Engine() {
    if (slEngineObj_ != nullptr) {
        (*slEngineObj_)->Destroy(slEngineObj_);
        slEngineObj_ = nullptr;
        slEngineItf_ = nullptr;
    }
}

void ConvertToSLSampleFormat(SLAndroidDataFormat_PCM_EX *pFormat, SampleFormat *pSampleInfo_) {
    assert(pFormat);
    memset(pFormat, 0, sizeof(*pFormat));

    pFormat->formatType = SL_DATAFORMAT_PCM;
    pFormat->numChannels = 1;
    pFormat->channelMask = SL_SPEAKER_FRONT_LEFT;
    pFormat->sampleRate = pSampleInfo_->sampleRate_;

    pFormat->endianness = SL_BYTEORDER_LITTLEENDIAN;
    pFormat->bitsPerSample = pSampleInfo_->pcmFormat_;
    pFormat->containerSize = pSampleInfo_->pcmFormat_;

    // fixup for android extended representations...
    pFormat->representation = pSampleInfo_->representation_;
    switch (pFormat->representation) {
        case SL_ANDROID_PCM_REPRESENTATION_UNSIGNED_INT:
            pFormat->bitsPerSample = SL_PCMSAMPLEFORMAT_FIXED_8;
            pFormat->containerSize = SL_PCMSAMPLEFORMAT_FIXED_8;
            pFormat->formatType = SL_ANDROID_DATAFORMAT_PCM_EX;
            break;
        case SL_ANDROID_PCM_REPRESENTATION_SIGNED_INT:
            pFormat->bitsPerSample = SL_PCMSAMPLEFORMAT_FIXED_16;  // supports 16, 24, and 32
            pFormat->containerSize = SL_PCMSAMPLEFORMAT_FIXED_16;
            pFormat->formatType = SL_ANDROID_DATAFORMAT_PCM_EX;
            break;
        case SL_ANDROID_PCM_REPRESENTATION_FLOAT:
            pFormat->bitsPerSample = SL_PCMSAMPLEFORMAT_FIXED_32;
            pFormat->containerSize = SL_PCMSAMPLEFORMAT_FIXED_32;
            pFormat->formatType = SL_ANDROID_DATAFORMAT_PCM_EX;
            break;
        case 0:
            break;
        default:
            assert(0);
    }
}

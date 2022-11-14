#include <cassert>
#include <cstdio>
#include <__threading_support>

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

#define RECORDER_FRAMES (16000 * 5)
static SLObjectItf audioEngineObject = nullptr;
static SLEngineItf audioEngine;
static SLObjectItf recorderObject = nullptr;
static SLRecordItf recorderRecord;
static short recorderBuffer[RECORDER_FRAMES];
static SLAndroidSimpleBufferQueueItf recorderBufferQueue;
static pthread_mutex_t audioEngineLock = PTHREAD_MUTEX_INITIALIZER;

/** This callback handler is called every time a buffer finishes recording. */
void bqRecorderCallback(SLAndroidSimpleBufferQueueItf bq, void *context) {
    assert(bq == recorderBufferQueue);
    assert(nullptr == context);
    (*recorderRecord)->SetRecordState(recorderRecord, SL_RECORDSTATE_STOPPED);
    pthread_mutex_unlock(&audioEngineLock);
}

int8_t createAudioRecorder() {
    SLresult result;

    // Create engine
    result = slCreateEngine(&audioEngineObject, 0, nullptr,
                            0, nullptr, nullptr);
    if (result != SL_RESULT_SUCCESS) return 1;
    (void) result;

    // Realize the engine
    result = (*audioEngineObject)->Realize(audioEngineObject, SL_BOOLEAN_FALSE);
    if (result != SL_RESULT_SUCCESS) return 2;
    (void) result;

    // Get the engine interface, which is needed in order to create other objects
    result = (*audioEngineObject)->GetInterface(audioEngineObject, SL_IID_ENGINE, &audioEngine);
    if (result != SL_RESULT_SUCCESS) return 3;
    (void) result;

    // Configure audio source
    SLDataLocator_IODevice loc_dev = {SL_DATALOCATOR_IODEVICE, SL_IODEVICE_AUDIOINPUT,
                                      SL_DEFAULTDEVICEID_AUDIOINPUT, nullptr};
    SLDataSource audioSrc = {&loc_dev, nullptr};

    // Configure audio sink
    SLDataLocator_AndroidSimpleBufferQueue loc_bq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
    SLDataFormat_PCM format_pcm = {SL_DATAFORMAT_PCM, 1, SL_SAMPLINGRATE_16,
                                   SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
                                   SL_SPEAKER_FRONT_CENTER, SL_BYTEORDER_LITTLEENDIAN};
    SLDataSink audioSnk = {&loc_bq, &format_pcm};

    // Create audio recorder (requires the RECORD_AUDIO permission)
    const SLInterfaceID id[1] = {SL_IID_ANDROIDSIMPLEBUFFERQUEUE};
    const SLboolean req[1] = {SL_BOOLEAN_TRUE};
    result = (*audioEngine)->CreateAudioRecorder(
            audioEngine, &recorderObject, &audioSrc, &audioSnk, 1, id, req);
    if (result != SL_RESULT_SUCCESS) return 4;

    // Realize the audio recorder
    result = (*recorderObject)->Realize(recorderObject, SL_BOOLEAN_FALSE);
    if (result != SL_RESULT_SUCCESS) return 5;

    // Get the record interface
    result = (*recorderObject)->GetInterface(recorderObject, SL_IID_RECORD, &recorderRecord);
    if (result != SL_RESULT_SUCCESS) return 6;
    (void) result;

    // Get the buffer queue interface
    result = (*recorderObject)->GetInterface(
            recorderObject, SL_IID_ANDROIDSIMPLEBUFFERQUEUE, &recorderBufferQueue);
    if (result != SL_RESULT_SUCCESS) return 7;
    (void) result;

    // Register callback on the buffer queue
    result = (*recorderBufferQueue)->RegisterCallback(
            recorderBufferQueue, bqRecorderCallback, nullptr);
    if (result != SL_RESULT_SUCCESS) return 8;
    (void) result;

    return 0;
}

int8_t startRecording() {
    if (recorderRecord == nullptr) return 1;
    SLresult result;
    if (pthread_mutex_trylock(&audioEngineLock)) return 2;

    // Enqueue an empty buffer to be filled by the recorder
    // (for streaming recording, we would enqueue at least 2 empty buffers to start things off)
    result = (*recorderBufferQueue)->Enqueue(
            recorderBufferQueue, recorderBuffer, RECORDER_FRAMES * sizeof(short));
    // The most likely other result is SL_RESULT_BUFFER_INSUFFICIENT,
    // which for this code example would indicate a programming error
    if (result != SL_RESULT_SUCCESS) return 3;
    (void) result;

    // Start recording
    result = (*recorderRecord)->SetRecordState(recorderRecord, SL_RECORDSTATE_RECORDING);
    if (result != SL_RESULT_SUCCESS) return 4;
    (void) result;

    return 0;
}

int8_t stopRecording() {
    if (recorderRecord == nullptr) return 1;
    SLresult result;

    result = (*recorderRecord)->SetRecordState(recorderRecord, SL_RECORDSTATE_STOPPED);
    if (result != SL_RESULT_SUCCESS) return 2;
    (void) result;

    // TODO SAVE

    // Clear buffer queue
    result = (*recorderBufferQueue)->Clear(recorderBufferQueue);
    if (result != SL_RESULT_SUCCESS) return 3;
    (void) result;

    return 0;
}

void deleteAudioRecorder() {
    // Destroy audio recorder object, and invalidate all associated interfaces
    if (recorderObject != nullptr) {
        (*recorderObject)->Destroy(recorderObject);
        recorderObject = nullptr;
        recorderRecord = nullptr;
        recorderBufferQueue = nullptr;
    }

    // Destroy engine object, and invalidate all associated interfaces
    if (audioEngineObject != nullptr) {
        (*audioEngineObject)->Destroy(audioEngineObject);
        audioEngineObject = nullptr;
        audioEngine = nullptr;
    }
}

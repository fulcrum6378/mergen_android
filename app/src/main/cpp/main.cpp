#include <cstring>
#include <jni.h>

#include "aud/audio_common.h"
#include "aud/audio_recorder.h"
#include "vis/recorder.cpp" // don't put these in namespace brackets.

struct EchoAudioEngine {
    SLmilliHertz fastPathSampleRate_;
    uint32_t fastPathFramesPerBuf_;
    uint16_t sampleChannels_;
    uint16_t bitsPerSample_;

    SLObjectItf slEngineObj_;
    SLEngineItf slEngineItf_;

    AudioRecorder *recorder_;
    AudioQueue *freeBufQueue_;  // Owner of the queue
    AudioQueue *recBufQueue_;   // Owner of the queue
    // recBufQueue_ is recQueue_ for the recorder and playQueue_ for the player.

    sample_buf *bufs_;
    uint32_t bufCount_;
};
static EchoAudioEngine engine;

extern "C" JNIEXPORT jlong JNICALL
Java_ir_mahdiparastesh_mergen_Main_prepare(JNIEnv *env, jobject, jint sampleRate,
                                           jint framesPerBuf) {
    //createAudioRecorder();
    SLresult result;
    memset(&engine, 0, sizeof(engine));

    engine.fastPathSampleRate_ = static_cast<SLmilliHertz>(sampleRate) * 1000;
    engine.fastPathFramesPerBuf_ = static_cast<uint32_t>(framesPerBuf);
    engine.sampleChannels_ = AUDIO_SAMPLE_CHANNELS;
    engine.bitsPerSample_ = SL_PCMSAMPLEFORMAT_FIXED_16;

    result = slCreateEngine(
            &engine.slEngineObj_, 0, nullptr, 0,
            nullptr, nullptr);
    SLASSERT(result);

    result = (*engine.slEngineObj_)->Realize(engine.slEngineObj_, SL_BOOLEAN_FALSE);
    SLASSERT(result);

    result = (*engine.slEngineObj_)->GetInterface(engine.slEngineObj_, SL_IID_ENGINE,
                                                  &engine.slEngineItf_);
    SLASSERT(result);

    // compute the RECOMMENDED fast audio buffer size:
    //   the lower latency required
    //     *) the smaller the buffer should be (adjust it here) AND
    //     *) the less buffering should be before starting player AFTER
    //        receiving the recorder buffer
    //   Adjust the bufSize here to fit your bill [before it busts]
    uint32_t bufSize = engine.fastPathFramesPerBuf_ * engine.sampleChannels_ *
                       engine.bitsPerSample_;
    bufSize = (bufSize + 7) >> 3;  // bits --> byte
    engine.bufCount_ = BUF_COUNT;
    engine.bufs_ = allocateSampleBufs(engine.bufCount_, bufSize);
    assert(engine.bufs_);
    engine.freeBufQueue_ = new AudioQueue(engine.bufCount_);
    engine.recBufQueue_ = new AudioQueue(engine.bufCount_);
    assert(engine.freeBufQueue_ && engine.recBufQueue_);
    for (uint32_t i = 0; i < engine.bufCount_; i++)
        engine.freeBufQueue_->push(&engine.bufs_[i]);

    SampleFormat sampleFormat{};
    memset(&sampleFormat, 0, sizeof(sampleFormat));
    sampleFormat.pcmFormat_ = static_cast<uint16_t>(engine.bitsPerSample_);
    sampleFormat.channels_ = engine.sampleChannels_;
    sampleFormat.sampleRate_ = engine.fastPathSampleRate_;
    sampleFormat.framesPerBuf_ = engine.fastPathFramesPerBuf_;
    engine.recorder_ = new AudioRecorder(&sampleFormat, engine.slEngineItf_);
    if (!engine.recorder_) return JNI_FALSE;
    engine.recorder_->SetBufQueues(engine.freeBufQueue_, engine.recBufQueue_);

    return createCamera(env);
}

extern "C" JNIEXPORT jbyte JNICALL
Java_ir_mahdiparastesh_mergen_Main_start(JNIEnv *, jobject) {
    //startRecording();
    engine.recorder_->Start();

    startStreaming();
    return 0;
}

extern "C" JNIEXPORT jbyte JNICALL
Java_ir_mahdiparastesh_mergen_Main_stop(JNIEnv *, jobject) {
    //stopRecording();
    engine.recorder_->Stop();
    delete engine.recorder_;
    engine.recorder_ = nullptr;

    stopStreaming();
    return 0;
}

extern "C" JNIEXPORT void JNICALL
Java_ir_mahdiparastesh_mergen_Main_destroy(JNIEnv *, jobject, jlong ndkCameraObj) {
    //deleteAudioRecorder();
    delete engine.recorder_;
    engine.recorder_ = nullptr;
    delete engine.recBufQueue_;
    delete engine.freeBufQueue_;
    releaseSampleBufs(engine.bufs_, engine.bufCount_);
    if (engine.slEngineObj_ != nullptr) {
        (*engine.slEngineObj_)->Destroy(engine.slEngineObj_);
        engine.slEngineObj_ = nullptr;
        engine.slEngineItf_ = nullptr;
    }

    deleteCamera(ndkCameraObj);
}

extern "C" JNIEXPORT jstring JNICALL
Java_ir_mahdiparastesh_mergen_Main_test(JNIEnv */*env*/, jobject) {
    /*#include <chrono>
    #include <ctime>
    std::chrono::system_clock::time_point ts = std::chrono::system_clock::now();
    ts.operator+=(std::chrono::duration<std::int64_t, std::ratio<31556952>>(1));
    std::tm* lc = std::gmtime(reinterpret_cast<const time_t *>(ts.time_since_epoch().count()));
    return env->NewStringUTF(std::to_string(lc->tm_year).c_str());*/
    return nullptr;
}

/**
 * @returns minimium camera preview window size for the given
 * requested camera size in CreateCamera() function, with the same
 * ascpect ratio. essentially,
 *   1) Display device size decides NDKCamera object preview size
 *   2) Chosen NDKCamera preview size passed to TextView to
 *      reset textureView size
 *   3) textureView size is stretched when previewing image
 *      on display device
 */
extern "C" JNIEXPORT jobject JNICALL
Java_ir_mahdiparastesh_mergen_Main_getMinimumCompatiblePreviewSize(
        JNIEnv *env, jobject, jlong ndkCameraObj) {
    if (!ndkCameraObj) return nullptr;
    auto *pApp = reinterpret_cast<CameraEngine *>(ndkCameraObj);
    jclass cls = env->FindClass("android/util/Size");
    jobject previewSize = env->NewObject(
            cls, env->GetMethodID(cls, "<init>", "(II)V"),
            pApp->GetCompatibleCameraRes().width,
            pApp->GetCompatibleCameraRes().height);
    return previewSize;
}

extern "C" JNIEXPORT void JNICALL
Java_ir_mahdiparastesh_mergen_Main_onSurfaceStatusChanged(
        JNIEnv *env, jobject, jboolean available, jlong ndkCameraObj, jobject surface) {
    if (available) onPreviewSurfaceCreated(ndkCameraObj, surface);
    else onPreviewSurfaceDestroyed(env, ndkCameraObj, surface);
}

// The idea of defining a JNI interface header sucks!

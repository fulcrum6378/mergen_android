#include <cstring>
#include <jni.h>

#include "aud/recorder.cpp"
#include "vis/recorder.cpp" // don't put these in namespace brackets.

extern "C" JNIEXPORT jlong JNICALL
Java_ir_mahdiparastesh_mergen_Main_prepare(JNIEnv *env, jobject) {
    createAudioRecorder();
    return createCamera(env);
}

extern "C" JNIEXPORT jbyte JNICALL
Java_ir_mahdiparastesh_mergen_Main_start(JNIEnv *, jobject) {
    startRecording();
    startStreaming();
    return 0;
}

extern "C" JNIEXPORT jbyte JNICALL
Java_ir_mahdiparastesh_mergen_Main_stop(JNIEnv *, jobject) {
    stopRecording();
    stopStreaming();
    return 0;
}

extern "C" JNIEXPORT void JNICALL
Java_ir_mahdiparastesh_mergen_Main_destroy(JNIEnv *, jobject, jlong ndkCameraObj) {
    deleteAudioRecorder();
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
    jobject previewSize =
            env->NewObject(cls, env->GetMethodID(cls, "<init>", "(II)V"),
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

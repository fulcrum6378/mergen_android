#include <android/native_window_jni.h>

#include "aud/engine.h"
#include "vis/camera.h"

static AudioEngine *aud = nullptr;
static Camera *vis = nullptr;

extern "C" JNIEXPORT jlong JNICALL
Java_ir_mahdiparastesh_mergen_Main_create(JNIEnv *, jobject, jint w, jint h) {
    aud = new AudioEngine();
    vis = new Camera(w, h);
    return reinterpret_cast<jlong>(vis);
}

extern "C" JNIEXPORT jbyte JNICALL
Java_ir_mahdiparastesh_mergen_Main_start(JNIEnv *, jobject) {
    int8_t ret = 0;
    if (aud) {
        if (!aud->StartRecording()) ret = 1;
    } else ret = 2;
    if (ret > 0) return ret;
    if (vis) {
        if (!vis->SetRecording(true)) ret = 3;
    } else ret = 4;
    return ret;
}

extern "C" JNIEXPORT jbyte JNICALL
Java_ir_mahdiparastesh_mergen_Main_stop(JNIEnv *, jobject) {
    int8_t ret = 0;
    if (aud) {
        if (!aud->StopRecording()) ret = 1;
    } else ret = 2;
    if (ret > 0) return ret;
    if (vis) {
        if (!vis->SetRecording(false)) ret = 3;
    } else ret = 4;
    return ret;
}

extern "C" JNIEXPORT void JNICALL
Java_ir_mahdiparastesh_mergen_Main_destroy(JNIEnv *, jobject) {
    aud = nullptr;
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
    auto *pApp = reinterpret_cast<Camera *>(ndkCameraObj);
    jclass cls = env->FindClass("android/util/Size");
    jobject previewSize = env->NewObject(
            cls, env->GetMethodID(cls, "<init>", "(II)V"),
            pApp->GetDimensions().first, pApp->GetDimensions().second);
    return previewSize;
}

extern "C" JNIEXPORT void JNICALL
Java_ir_mahdiparastesh_mergen_Main_onSurfaceStatusChanged(
        JNIEnv *env, jobject, jboolean available, jlong ndkCameraObj, jobject surface) {
    auto *cam = reinterpret_cast<Camera *>(ndkCameraObj);
    ASSERT(ndkCameraObj && vis == cam,
           "%s", ("NativeObject should not be null Pointer: " +
                  std::to_string(ndkCameraObj) + " == " +
                  std::to_string(reinterpret_cast<jlong>(vis))).c_str())
    if (available) cam->CreateSession(ANativeWindow_fromSurface(env, surface));
    else { // don't put these in Main.destroy()
        delete cam;
        vis = nullptr; // also reset the private global object
    }
}

/* TODO:
  * Problems:
  * Use the latest image for mirroring while saving all frames, so that the preview won't backward.
  *
  * Notes:
  * The idea of defining a JNI interface header sucks!
  * Beware that AImageReader_acquireLatestImage deletes the previous images.
  * Use AImage_getTimestamp().
  * ACAMERA_LENS_FACING_BACK
  */

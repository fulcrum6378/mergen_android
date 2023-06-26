#include <android/native_window_jni.h>

#include "aud/engine.h"
#include "vis/camera.h"

static AudioEngine *aud = nullptr;
static Camera *vis = nullptr;
// static Queuer *mem = nullptr; #include "mem/queuer.h"

extern "C" JNIEXPORT jlong JNICALL
Java_ir_mahdiparastesh_mergen_Main_create(JNIEnv *, jobject) {
    //mem = new Queuer();
    aud = new AudioEngine(/*mem*/nullptr);
    vis = new Camera();
    // ComputeVK().run(state->activity->assetManager);

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
    /*delete &mem;
    mem = nullptr;*/
}


extern "C" JNIEXPORT jstring JNICALL
Java_ir_mahdiparastesh_mergen_Main_test(JNIEnv */*env*/, jobject) {
    /*jlong testShake = 200;
    jmethodID shakeMethod = env->GetMethodID(
            env->FindClass("ir/mahdiparastesh/mergen/Main"), "shake", "(J)V");
    env->CallVoidMethod(main, shakeMethod, testShake);*/
    return nullptr;
}

extern "C" JNIEXPORT jobject JNICALL
Java_ir_mahdiparastesh_mergen_Main_getCameraDimensions(
        JNIEnv *env, jobject, jlong cameraObj) {
    if (!cameraObj) return nullptr;
    auto *cam = reinterpret_cast<Camera *>(cameraObj);
    jclass cls = env->FindClass("android/util/Size");
    jobject previewSize = env->NewObject(
            cls, env->GetMethodID(cls, "<init>", "(II)V"),
            cam->GetDimensions().first, cam->GetDimensions().second);
    return previewSize;
}

extern "C" JNIEXPORT void JNICALL
Java_ir_mahdiparastesh_mergen_Main_onSurfaceStatusChanged(
        JNIEnv *env, jobject, jlong cameraObj, jobject surface, jboolean available) {
    auto *cam = reinterpret_cast<Camera *>(cameraObj);
    assert(cam == vis);
    if (available) cam->CreateSession(ANativeWindow_fromSurface(env, surface));
    else { // don't put these in Main.destroy()
        delete cam;
        vis = nullptr;
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
  * LET'S TEMPORARILY LEAVE "MEM" AND MERGE ITS JOB ALONG WITH "REW" INTO "TNK"!
  */

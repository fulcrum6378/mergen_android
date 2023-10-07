#include <android/native_window_jni.h>
#include <sys/stat.h>

#include "aud/microphone.h"
#include "hpt/touchscreen.h"
#include "rew/rewarder.h"
#include "vis/camera.h"

static Camera *vis = nullptr;
static Microphone *aud = nullptr;
static Touchscreen *hpt = nullptr;

static Rewarder *rew = nullptr;

extern "C" JNIEXPORT jlong JNICALL
Java_ir_mahdiparastesh_mergen_Main_create(JNIEnv *env, jobject main) {
    // retrieve necessary JNI references
    JavaVM *jvm = nullptr;
    env->GetJavaVM(&jvm);
    jobject gMain = env->NewGlobalRef(main);
    jmethodID mCaptured = env->GetMethodID(
            env->FindClass("ir/mahdiparastesh/mergen/Main"), "captured", "()V");

    // ensure that /files/ dir exists
    const char *filesDir = "/data/data/ir.mahdiparastesh.mergen/files/";
    struct stat sb;
    if (stat(filesDir, &sb) != 0) mkdir(filesDir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

    Manifest::init();
    rew = new Rewarder(env, gMain); // must be declared before the rest
    // ComputeVK().run(state->activity->assetManager);

    vis = new Camera(jvm, gMain, mCaptured);
    aud = new Microphone();
    hpt = new Touchscreen(rew);

    return reinterpret_cast<jlong>(vis);
}

extern "C" JNIEXPORT jbyte JNICALL
Java_ir_mahdiparastesh_mergen_Main_start(JNIEnv *, jobject) {
    int8_t ret = 0;
    if (aud) {
        if (!aud->Start()) ret = 1;
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
        if (!aud->Stop()) ret = 1;
    } else ret = 2;
    if (ret > 0) return ret;
    if (vis) {
        if (!vis->SetRecording(false)) ret = 3;
    } else ret = 4;
    return ret;
}

extern "C" JNIEXPORT void JNICALL
Java_ir_mahdiparastesh_mergen_Main_destroy(JNIEnv *, jobject) {
    delete aud;
    aud = nullptr;

    delete rew;
    rew = nullptr;
}


extern "C" JNIEXPORT jstring JNICALL
Java_ir_mahdiparastesh_mergen_Main_test(JNIEnv */*env*/, jobject) {
    return nullptr;
}

extern "C" JNIEXPORT jobject JNICALL
Java_ir_mahdiparastesh_mergen_Main_getCameraDimensions(
        JNIEnv *env, jobject, jlong cameraObj) {
    if (!cameraObj) return nullptr;
    jclass cls = env->FindClass("android/util/Size");
    auto dim = reinterpret_cast<Camera *>(cameraObj)->dimensions;
    jobject previewSize = env->NewObject(
            cls, env->GetMethodID(cls, "<init>", "(II)V"),
            dim.first, dim.second);
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

extern "C" JNIEXPORT void JNICALL
Java_ir_mahdiparastesh_mergen_Main_onTouch(
        JNIEnv *, jobject, jint dev, jint act, jint id, jfloat x, jfloat y, jfloat size) {
    if (!hpt) return;
    hpt->OnTouchEvent(dev, act, id, x, y, size);
}

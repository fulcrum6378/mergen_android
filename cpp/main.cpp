#include <android/native_window_jni.h>

#include "aud/microphone.h"
#include "hpt/touch.h"
#include "rew/rewarder.h"
#include "vis/camera.h"

// static Queuer *mem = nullptr; #include "mem/queuer.h"
static Rewarder *rew = nullptr;

static Microphone *aud = nullptr;
static Camera *vis = nullptr;

extern "C" JNIEXPORT jlong JNICALL
Java_ir_mahdiparastesh_mergen_Main_create(JNIEnv *, jobject) {
    Manifest::create();
    rew = new Rewarder();
    // mem = new Queuer();
    // ComputeVK().run(state->activity->assetManager);

    aud = new Microphone(/*mem*/nullptr);
    vis = new Camera();

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
    aud = nullptr;
    /*delete &mem;
    mem = nullptr;*/
    delete &rew;
    rew = nullptr;
    Manifest::destroy();
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

#include <sstream>

extern "C" JNIEXPORT void JNICALL
Java_ir_mahdiparastesh_mergen_Main_onTouch(
        JNIEnv *, jobject, jint action, jfloat x, jfloat y, jfloat size) {
    std::stringstream ss;
    switch (action) {
        case 0:
            ss << "ACTION_DOWN";
            break;
        case 1:
            ss << "ACTION_UP";
            break;
        case 2:
            ss << "ACTION_MOVE";
            break;
        case 3:
            ss << "ACTION_CANCEL";
            break;
        default:
            ss << "UNKNOWN{" << action << "}";
    }
    ss << ", " << x << ", " << y << ", " << size;
    LOGW("%s", ss.str().c_str());
}

#include <android/native_window_jni.h>
#include <jni.h>
#include <sys/stat.h>

#include "aud/microphone.h"
#include "hpt/touchscreen.h"
#include "rew/rewarder.h"
#include "scm/crawler.h"
#include "vis/camera.h"

static Crawler *scm = nullptr;
static Rewarder *rew = nullptr;

static Camera *vis = nullptr;
static Microphone *aud = nullptr; // temporarily disabled
static Touchscreen *hpt = nullptr;

extern "C" JNIEXPORT jlong JNICALL
Java_ir_mahdiparastesh_mergen_Main_create(JNIEnv *env, jobject main) {
    // retrieve necessary JNI references
    JavaVM *jvm = nullptr;
    env->GetJavaVM(&jvm);
    jobject gMain = env->NewGlobalRef(main);

    // ensure that /files/ dir exists
    const char *filesDir = "/data/data/ir.mahdiparastesh.mergen/files/";
    struct stat sb{};
    if (stat(filesDir, &sb) != 0) mkdir(filesDir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

    // initialise high-level parts
    Manifest::init();
    scm = new Crawler();
    rew = new Rewarder(env, gMain); // must be declared before the rest
    // ComputeVK().run(state->activity->assetManager);

    // initialise low-level parts
    vis = new Camera(scm->vis, jvm, gMain);
    aud = new Microphone();
    hpt = new Touchscreen(rew);

    return reinterpret_cast<jlong>(vis);
}

extern "C" JNIEXPORT jbyte JNICALL
Java_ir_mahdiparastesh_mergen_Main_start(JNIEnv *, jobject, jbyte debugMode) {
    int8_t ret = 0;
    /*if (aud) {
        if (!aud->Start()) ret = 1;
    } else ret = 2;
    if (ret > 0) return ret;*/
    if (vis) {
        if (!vis->SetRecording(true, debugMode)) ret = 3;
    } else ret = 4;
    return ret;
}

extern "C" JNIEXPORT jbyte JNICALL
Java_ir_mahdiparastesh_mergen_Main_stop(JNIEnv *, jobject) {
    int8_t ret = 0;
    /*if (aud) {
        if (!aud->Stop()) ret = 1;
    } else ret = 2;
    if (ret > 0) return ret;*/
    if (vis) {
        if (!vis->SetRecording(false, 0)) ret = 3;
    } else ret = 4;
    return ret;
}

extern "C" JNIEXPORT void JNICALL
Java_ir_mahdiparastesh_mergen_Main_destroy(JNIEnv *, jobject) {
    delete aud;
    aud = nullptr;

    delete rew;
    rew = nullptr;
    delete scm;
    scm = nullptr;
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
    hpt->OnTouchEvent((int16_t) dev, (int8_t) act, (int16_t) id, x, y, size);
}

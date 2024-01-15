#include <android/native_window_jni.h>
#include <jni.h>
#include <sys/stat.h>

#include "aud/microphone.hpp"
#include "aud/speaker.hpp"
#include "hpt/touchscreen.hpp"
#include "mov/vibrator.hpp"
#include "rew/rewarder.hpp"
#include "scm/manifest.hpp"
#include "scm/perception.hpp"
#include "vis/camera.hpp"

static Perception *scm;
static Rewarder *rew;

static Microphone *aud_in;
static Speaker *aud_out;
static Touchscreen *hpt;
static Vibrator *mov;
static Camera *vis;
static ANativeWindow *vis_dbg;

extern "C" JNIEXPORT void JNICALL
Java_ir_mahdiparastesh_mergen_Main_create(JNIEnv *env, jobject main) {
    // retrieve necessary JNI references (don't put them in static variables)
    JavaVM *jvm = nullptr;
    env->GetJavaVM(&jvm);
    jobject gMain = env->NewGlobalRef(main);

    // ensure that /files/ and its required subdirectories exist
    struct stat sb{};
    for (std::string dirN: {""/*, "aud"*//*, "hpt"*/, "vis"}) {
        const char *dir = (filesDir + dirN).c_str();
        if (stat(dir, &sb) != 0)
            mkdir(dir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    }

    // initialise low-level components
    aud_in = new Microphone();
    aud_out = new Speaker();
    hpt = new Touchscreen();
    mov = new Vibrator(jvm, gMain);
    vis = new Camera(jvm, gMain, &vis_dbg);

    // initialise high-level components
    rew = new Rewarder(aud_out, mov, jvm, gMain);
    Manifest::init();
    scm = new Perception();
}

extern "C" JNIEXPORT jbyte JNICALL
Java_ir_mahdiparastesh_mergen_Main_start(JNIEnv *, jobject, jbyte debugMode) {
    int8_t ret = 0;
    if (!aud_in->Start()) ret = 1;
    if (ret != 0) return ret;
    if (!vis->SetRecording(true, debugMode)) ret = 2;
    return ret;
}

extern "C" JNIEXPORT jbyte JNICALL
Java_ir_mahdiparastesh_mergen_Main_stop(JNIEnv *, jobject) {
    int8_t ret = 0;
    if (!aud_in->Stop()) ret = 1;
    if (ret != 0) return ret;
    if (!vis->SetRecording(false, 0)) ret = 2;
    return ret;
}

extern "C" JNIEXPORT void JNICALL
Java_ir_mahdiparastesh_mergen_Main_destroy(JNIEnv *, jobject) {
    delete aud_in;
    aud_in = nullptr;
    delete aud_out;
    aud_out = nullptr;
    delete hpt;
    hpt = nullptr;
    delete mov;
    mov = nullptr;

    delete rew;
    rew = nullptr;
    delete scm;
    scm = nullptr;
}


extern "C" JNIEXPORT jobject JNICALL
Java_ir_mahdiparastesh_mergen_Main_getCameraDimensions(JNIEnv *env, jobject) {
    jclass cls = env->FindClass("android/util/Size");
    return env->NewObject(
            cls, env->GetMethodID(cls, "<init>", "(II)V"),
            vis->dimensions.first, vis->dimensions.second);
}

extern "C" JNIEXPORT void JNICALL
Java_ir_mahdiparastesh_mergen_Main_onPreviewSurfaceCreated(
        JNIEnv *env, jobject, jobject surface) {
    vis->CreateSession(ANativeWindow_fromSurface(env, surface));
}

extern "C" JNIEXPORT void JNICALL
Java_ir_mahdiparastesh_mergen_Main_onPreviewSurfaceDestroyed(JNIEnv *, jobject) {
    // don't put these in Main.destroy()
    delete vis;
    vis = nullptr;
}

extern "C" JNIEXPORT void JNICALL
Java_ir_mahdiparastesh_mergen_Main_onAnalysesSurfaceCreated(JNIEnv *env, jobject, jobject surface) {
    vis_dbg = ANativeWindow_fromSurface(env, surface);
    ANativeWindow_setBuffersGeometry(
            vis_dbg, W, H, AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM);
}

extern "C" JNIEXPORT void JNICALL
Java_ir_mahdiparastesh_mergen_Main_onAnalysesSurfaceDestroyed(JNIEnv *, jobject) {
    vis_dbg = nullptr;
}

extern "C" JNIEXPORT void JNICALL
Java_ir_mahdiparastesh_mergen_Main_onTouch(
        JNIEnv *, jobject, jint dev, jint act, jint id, jfloat x, jfloat y, jfloat size) {
    if (!hpt) return;
    hpt->OnTouchEvent((int16_t) dev, (int8_t) act, (int16_t) id, x, y, size);
}

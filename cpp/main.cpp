#include <android/asset_manager_jni.h>
#include <android/native_window_jni.h>
#include <cassert>
#include <jni.h>
#include <sys/stat.h>

#include "aud/microphone.hpp"
#include "aud/speaker.hpp"
#include "hpt/touchscreen.hpp"
#include "mov/vibrator.hpp"
#include "rew/rewarder.hpp"
#include "scm/perception.hpp"
#include "vis/camera.hpp"
#include "vlk/hello_vk.hpp"

static Perception *scm;
static Rewarder *rew;
static HelloVK *vlk;

static Microphone *aud_in;
static Speaker *aud_out;
static Touchscreen *hpt;
static Vibrator *mov;
static Camera *vis;

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
    vis = new Camera(jvm, gMain);

    // initialise high-level components
    Manifest::init();
    rew = new Rewarder(aud_out, mov, jvm, gMain);
    scm = new Perception();
    // ComputeVK().run(state->activity->assetManager);
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
Java_ir_mahdiparastesh_mergen_Main_onSurfaceStatusChanged(
        JNIEnv *env, jobject, jobject surface, jboolean available, jobject assetManager) {
    if (available) {
        //vis->CreateSession(ANativeWindow_fromSurface(env, surface));
        vlk = new HelloVK();
        vlk->reset(ANativeWindow_fromSurface(env, surface),
                   AAssetManager_fromJava(env, assetManager));
        vlk->initVulkan();
        vlk->render();
    } else { // don't put these in Main.destroy()
        //delete vis;
        //vis = nullptr;
        vlk->cleanup();
        delete vlk;
        vlk = nullptr;
    }
}

extern "C" JNIEXPORT void JNICALL
Java_ir_mahdiparastesh_mergen_Main_onTouch(
        JNIEnv *, jobject, jint dev, jint act, jint id, jfloat x, jfloat y, jfloat size) {
    if (!hpt) return;
    hpt->OnTouchEvent((int16_t) dev, (int8_t) act, (int16_t) id, x, y, size);
}

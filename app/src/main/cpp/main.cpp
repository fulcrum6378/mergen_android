#include <cstring>
#include <jni.h>

namespace AUD {

#include "aud/recorder.cpp"

}
namespace VIS {

#include "vis/recorder.cpp"

}

extern "C" JNIEXPORT jlong JNICALL
Java_ir_mahdiparastesh_mergen_Main_prepare(JNIEnv *env, jobject,
                                           jint visWidth, jint visHeight) {
    AUD::createAudioRecorder();
    return VIS::createCamera(env, visWidth, visHeight);
}

extern "C" JNIEXPORT jbyte JNICALL
Java_ir_mahdiparastesh_mergen_Main_start(JNIEnv *, jobject) {
    VIS::startRecording();
    AUD::startRecording();
    return 0;
}

extern "C" JNIEXPORT jbyte JNICALL
Java_ir_mahdiparastesh_mergen_Main_stop(JNIEnv *, jobject) {
    VIS::stopRecording();
    AUD::stopRecording();
    return 0;
}

extern "C" JNIEXPORT void JNICALL
Java_ir_mahdiparastesh_mergen_Main_destroy(JNIEnv *, jobject, jlong ndkCameraObj) {
    AUD::deleteAudioRecorder();
    VIS::deleteCamera(ndkCameraObj);
}

extern "C" JNIEXPORT jstring JNICALL
Java_ir_mahdiparastesh_mergen_Main_test(JNIEnv *, jobject) {
    /*#include <chrono>
    #include <ctime>
    std::chrono::system_clock::time_point ts = std::chrono::system_clock::now();
    ts.operator+=(std::chrono::duration<std::int64_t, std::ratio<31556952>>(1));
    std::tm* lc = std::gmtime(reinterpret_cast<const time_t *>(ts.time_since_epoch().count()));
    return env->NewStringUTF(std::to_string(lc->tm_year).c_str());*/
    return nullptr;
}

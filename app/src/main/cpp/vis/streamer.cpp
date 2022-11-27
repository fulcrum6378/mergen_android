#include "camera_engine.h"
#include "ndk_camera.h"
#include "../otr/debug.h"

static CameraEngine *cameraEngine = nullptr;

static jlong createCamera(JNIEnv *env, jint w, jint h) {
    cameraEngine = new CameraEngine(env, w, h);
    return reinterpret_cast<jlong>(cameraEngine);
}

static void deleteCamera(jlong ndkCameraObj) {
    if (!cameraEngine || !ndkCameraObj) return;
    auto *pApp = reinterpret_cast<CameraEngine *>(ndkCameraObj);
    ASSERT(pApp == cameraEngine, "NdkCamera Obj mismatch")

    delete pApp;

    // also reset the private global object
    cameraEngine = nullptr;
}

/*extern "C" JNIEXPORT jint JNICALL
Java_ir_mahdiparastesh_mergen_Main_getCameraSensorOrientation(
        JNIEnv *, jobject, jlong ndkCameraObj) {
    ASSERT(ndkCameraObj, "NativeObject should not be null Pointer")
    auto *pApp = reinterpret_cast<CameraEngine *>(ndkCameraObj);
    return pApp->GetCameraSensorOrientation(ACAMERA_LENS_FACING_BACK);
}*/

static void onPreviewSurfaceCreated(JNIEnv *env, jlong ndkCameraObj, jobject surface) {
    ASSERT(ndkCameraObj && (jlong) cameraEngine == ndkCameraObj,
           "NativeObject should not be null Pointer")
    auto *pApp = reinterpret_cast<CameraEngine *>(ndkCameraObj);
    pApp->CreateCameraSession(env, surface);
    pApp->StartPreview(true);
}

static void onPreviewSurfaceDestroyed(JNIEnv *env, jlong ndkCameraObj, jobject surface) {
    auto *pApp = reinterpret_cast<CameraEngine *>(ndkCameraObj);
    ASSERT(ndkCameraObj && cameraEngine == pApp,
           "%s", ("NativeObject should not be null Pointer: " +
                  std::to_string(ndkCameraObj) + " == " +
                  std::to_string(reinterpret_cast<jlong>(cameraEngine))).c_str())
    jclass cls = env->FindClass("android/view/Surface");
    jmethodID toString =
            env->GetMethodID(cls, "toString", "()Ljava/lang/String;");

    auto destroyObjStr =
            reinterpret_cast<jstring>(env->CallObjectMethod(surface, toString));
    const char *destroyObjName = env->GetStringUTFChars(destroyObjStr, nullptr);

    auto appObjStr = reinterpret_cast<jstring>(
            env->CallObjectMethod(pApp->GetSurfaceObject(), toString));
    const char *appObjName = env->GetStringUTFChars(appObjStr, nullptr);

    ASSERT(!strcmp(destroyObjName, appObjName), "object Name MisMatch")

    env->ReleaseStringUTFChars(destroyObjStr, destroyObjName);
    env->ReleaseStringUTFChars(appObjStr, appObjName);

    pApp->StartPreview(false);

    deleteCamera(ndkCameraObj); // Don't put this in Main.destroy()
}

static int8_t startStreaming() {
    if (cameraEngine == nullptr) return 1;
    cameraEngine->SetRecording(true);
    return 0;
}

static int8_t stopStreaming() {
    if (cameraEngine == nullptr) return 1;
    cameraEngine->SetRecording(false);
    return 0;
}

/*JNIEnv *jni;
  app_->activity->vm->AttachCurrentThread(&jni, NULL);

  // Default class retrieval
  jclass clazz = jni->GetObjectClass(app_->activity->clazz);
  jmethodID methodID = jni->GetMethodID(clazz, "OnPhotoTaken", "(Ljava/lang/String;)V");
  jstring javaName = jni->NewStringUTF(fileName);

  jni->CallVoidMethod(app_->activity->clazz, methodID, javaName);
  app_->activity->vm->DetachCurrentThread();*/

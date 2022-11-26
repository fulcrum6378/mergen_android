#include "camera_engine.h"
#include "ndk_camera.h"
#include "../otr/debug.h"

static CameraEngine *cameraEngine = nullptr;

/**
 * createCamera() Create application instance and NDK camera object
 * @param  width is the texture view window width
 * @param  height is the texture view window height
 * In this sample, it takes the simplest approach in that:
 * the android system display size is used to view full screen
 * preview camera images. Onboard Camera most likely to
 * support the onboard panel size on that device. Camera is most likely
 * to be oriented as landscape mode, the input width/height is in
 * landscape mode. TextureView on Java side handles rotation for
 * portrait mode.
 * @return application object instance ( not used in this sample )
 */
static jlong createCamera(JNIEnv *env, jint w, jint h) {
    cameraEngine = new CameraEngine(env, w, h);
    return reinterpret_cast<jlong>(cameraEngine);
}

/**
 * deleteCamera():
 *   releases native application object, which
 *   triggers native camera object be released
 */
static void deleteCamera(jlong ndkCameraObj) {
    if (!cameraEngine || !ndkCameraObj) return;
    auto *pApp = reinterpret_cast<CameraEngine *>(ndkCameraObj);
    ASSERT(pApp == cameraEngine, "NdkCamera Obj mismatch")

    delete pApp;

    // also reset the private global object
    cameraEngine = nullptr;
}

/**
 * getCameraSensorOrientation()
 * @ return camera sensor orientation angle relative to Android device's
 * display orientation. This sample only deal to back facing camera.
 */
/*extern "C" JNIEXPORT jint JNICALL
Java_ir_mahdiparastesh_mergen_Main_getCameraSensorOrientation(
        JNIEnv *, jobject, jlong ndkCameraObj) {
    ASSERT(ndkCameraObj, "NativeObject should not be null Pointer")
    auto *pApp = reinterpret_cast<CameraEngine *>(ndkCameraObj);
    return pApp->GetCameraSensorOrientation(ACAMERA_LENS_FACING_BACK);
}*/

/**
 * OnPreviewSurfaceCreated()
 *   Notification to native camera that java TextureView is ready
 *   to preview video. Simply create cameraSession and
 *   start camera preview
 */
static void onPreviewSurfaceCreated(jlong ndkCameraObj, jobject surface) {
    ASSERT(ndkCameraObj && (jlong) cameraEngine == ndkCameraObj,
           "NativeObject should not be null Pointer")
    auto *pApp = reinterpret_cast<CameraEngine *>(ndkCameraObj);
    pApp->CreateCameraSession(surface);
    pApp->StartPreview(true);
}

/**
 * OnPreviewSurfaceDestroyed()
 *   Notification to native camera that java TextureView is destroyed
 *   Native camera would:
 *      * stop preview
 */
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

    // Don't put this in Main.destroy()
    deleteCamera(ndkCameraObj);
}

static int8_t startStreaming() {
    if (cameraEngine == nullptr) return 1;
    // TODO start gathering photos through CameraEngine, through NDKCamera
    return 0;
}

static int8_t stopStreaming() {
    if (cameraEngine == nullptr) return 1;
    // TODO
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

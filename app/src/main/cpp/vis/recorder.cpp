#include <cstdint>

#include "camera_engine.h"
#include "camera_manager.h"
#include "../native_debug.h"

CameraEngine *cameraEngine = nullptr;

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
jlong createCamera(JNIEnv *env, jint width, jint height) {
    cameraEngine = new CameraEngine(env, width, height);
    return reinterpret_cast<jlong>(cameraEngine);
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
 * getCameraCompatibleSize()
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
    auto *pApp = reinterpret_cast<CameraEngine *>(ndkCameraObj);
    jclass cls = env->FindClass("android/util/Size");
    jobject previewSize =
            env->NewObject(cls, env->GetMethodID(cls, "<init>", "(II)V"),
                           pApp->GetCompatibleCameraRes().width,
                           pApp->GetCompatibleCameraRes().height);
    return previewSize;
}

/**
 * OnPreviewSurfaceCreated()
 *   Notification to native camera that java TextureView is ready
 *   to preview video. Simply create cameraSession and
 *   start camera preview
 */
extern "C" JNIEXPORT void JNICALL
Java_ir_mahdiparastesh_mergen_Main_onPreviewSurfaceCreated(
        JNIEnv *, jobject, jlong ndkCameraObj, jobject surface) {
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
extern "C" JNIEXPORT void JNICALL
Java_ir_mahdiparastesh_mergen_Main_onPreviewSurfaceDestroyed(
        JNIEnv *env, jobject, jlong ndkCameraObj, jobject surface) {
    auto *pApp = reinterpret_cast<CameraEngine *>(ndkCameraObj);
    ASSERT(ndkCameraObj && cameraEngine == pApp, "NativeObject should not be null Pointer")
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
}

int8_t startRecording() {
    if (cameraEngine == nullptr) return 1;
    // TODO start gathering photos through CameraEngine, through NDKCamera
    return 0;
}

int8_t stopRecording() {
    if (cameraEngine == nullptr) return 1;
    // TODO
    return 0;
}

/**
 * deleteCamera():
 *   releases native application object, which
 *   triggers native camera object be released
 */
void deleteCamera(jlong ndkCameraObj) {
    if (!cameraEngine || !ndkCameraObj) return;
    auto *pApp = reinterpret_cast<CameraEngine *>(ndkCameraObj);
    ASSERT(pApp == cameraEngine, "NdkCamera Obj mismatch")

    delete pApp;

    // also reset the private global object
    cameraEngine = nullptr;
}

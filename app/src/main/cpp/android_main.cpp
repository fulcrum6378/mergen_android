/* Description
 *     Demonstrate NDKCamera's PREVIEW mode -- hooks camera directly into
 *     display.
 *     Control:
 *         Double-Tap on Android device's screen to toggle start/stop preview
 *     Tested:
 *         Google Pixel and Nexus 6 phones
 */
#include <jni.h>
#include <cstring>
#include <utils/native_debug.h>
#include "camera_manager.h"
#include "camera_engine.h"

/**
 * Application object:
 *   the top level camera application object, maintained by native code only
 */
CameraAppEngine *pEngineObj = nullptr;

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
extern "C" JNIEXPORT jlong JNICALL
Java_ir_mahdiparastesh_mergen_Main_createCamera(JNIEnv *env,
                                                      jobject instance,
                                                      jint width, jint height) {
  pEngineObj = new CameraAppEngine(env, instance, width, height);
  return reinterpret_cast<jlong>(pEngineObj);
}

/**
 * deleteCamera():
 *   releases native application object, which
 *   triggers native camera object be released
 */
extern "C" JNIEXPORT void JNICALL
Java_ir_mahdiparastesh_mergen_Main_deleteCamera(JNIEnv *env,
                                                      jobject instance,
                                                      jlong ndkCameraObj, jobject surface) {
  if (!pEngineObj || !ndkCameraObj) {
    return;
  }
  auto *pApp = reinterpret_cast<CameraAppEngine *>(ndkCameraObj);
  ASSERT(pApp == pEngineObj, "NdkCamera Obj mismatch");

  delete pApp;

  // also reset the private global object
  pEngineObj = nullptr;
}

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
    JNIEnv *env, jobject instance, jlong ndkCameraObj) {
  if (!ndkCameraObj) {
    return nullptr;
  }
  auto *pApp = reinterpret_cast<CameraAppEngine *>(ndkCameraObj);
  jclass cls = env->FindClass("android/util/Size");
  jobject previewSize =
      env->NewObject(cls, env->GetMethodID(cls, "<init>", "(II)V"),
                     pApp->GetCompatibleCameraRes().width,
                     pApp->GetCompatibleCameraRes().height);
  return previewSize;
}

/**
 * getCameraSensorOrientation()
 * @ return camera sensor orientation angle relative to Android device's
 * display orientation. This sample only deal to back facing camera.
 */
extern "C" JNIEXPORT jint JNICALL
Java_ir_mahdiparastesh_mergen_Main_getCameraSensorOrientation(
    JNIEnv *env, jobject instance, jlong ndkCameraObj) {
  ASSERT(ndkCameraObj, "NativeObject should not be null Pointer");
  auto *pApp = reinterpret_cast<CameraAppEngine *>(ndkCameraObj);
  return pApp->GetCameraSensorOrientation(ACAMERA_LENS_FACING_BACK);
}

/**
 * OnPreviewSurfaceCreated()
 *   Notification to native camera that java TextureView is ready
 *   to preview video. Simply create cameraSession and
 *   start camera preview
 */
extern "C" JNIEXPORT void JNICALL
Java_ir_mahdiparastesh_mergen_Main_onPreviewSurfaceCreated(
    JNIEnv *env, jobject instance, jlong ndkCameraObj, jobject surface) {
  ASSERT(ndkCameraObj && (jlong)pEngineObj == ndkCameraObj,
         "NativeObject should not be null Pointer");
  auto *pApp = reinterpret_cast<CameraAppEngine *>(ndkCameraObj);
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
    JNIEnv *env, jobject instance, jlong ndkCameraObj, jobject surface) {
  auto *pApp = reinterpret_cast<CameraAppEngine *>(ndkCameraObj);
  ASSERT(ndkCameraObj && pEngineObj == pApp,
         "NativeObject should not be null Pointer");
  jclass cls = env->FindClass("android/view/Surface");
  jmethodID toString =
      env->GetMethodID(cls, "toString", "()Ljava/lang/String;");

  auto destroyObjStr =
      reinterpret_cast<jstring>(env->CallObjectMethod(surface, toString));
  const char *destroyObjName = env->GetStringUTFChars(destroyObjStr, nullptr);

  auto appObjStr = reinterpret_cast<jstring>(
      env->CallObjectMethod(pApp->GetSurfaceObject(), toString));
  const char *appObjName = env->GetStringUTFChars(appObjStr, nullptr);

  ASSERT(!strcmp(destroyObjName, appObjName), "object Name MisMatch");

  env->ReleaseStringUTFChars(destroyObjStr, destroyObjName);
  env->ReleaseStringUTFChars(appObjStr, appObjName);

  pApp->StartPreview(false);
}

#include "camera_engine.h"
#include "camera_manager.h"
#include "../native_debug.h"

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
static jlong createCamera(JNIEnv *env) {
    cameraEngine = new CameraEngine(env);
    return reinterpret_cast<jlong>(cameraEngine);
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

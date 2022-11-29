#include <android/native_window_jni.h>

#include "camera.h"
#include "../global.h"

Camera::Camera(JNIEnv *env, jint w, jint h)
        : env_(env),
          surface_(nullptr),
          cameraMgr_(nullptr),
          window_(nullptr),
          sessionOutput_(nullptr),
          target_(nullptr),
          request_(nullptr),
          outputContainer_(nullptr),
          captureSession_(nullptr),
          captureSessionState_(CaptureSessionState::MAX_STATE) {
    dimensions_ = std::pair<int32_t, int32_t>(w, h);
    MatchCaptureSizeRequest(&dimensions_);
    reader_ = new ImageReader(&dimensions_);

    cameras_.clear();
    cameraMgr_ = ACameraManager_create();
    ASSERT(cameraMgr_, "Failed to create cameraManager")

    // Pick up a back-facing camera to preview
    EnumerateCamera();
    ASSERT(!activeCameraId_.empty(), "Unknown ActiveCameraIdx")

    // Create back facing camera device
    ACameraManager_openCamera(cameraMgr_, activeCameraId_.c_str(),
                              GetDeviceListener(), &cameras_[activeCameraId_].device_);

    ACameraManager_registerAvailabilityCallback(cameraMgr_, GetManagerListener());

    // Initialize camera controls(exposure time and sensitivity), pick
    // up value of 2% * range + min as starting value (just a number, no magic)
    ACameraMetadata *metadataObj;
    ACameraManager_getCameraCharacteristics(
            cameraMgr_, activeCameraId_.c_str(), &metadataObj);
}

Camera::~Camera() {
    if (captureSessionState_ == CaptureSessionState::ACTIVE)
        ACameraCaptureSession_stopRepeating(captureSession_);
    ACameraCaptureSession_close(captureSession_);

    if (window_) {
        ACaptureRequest_removeTarget(request_, target_);
        ACaptureRequest_free(request_);
        ACameraOutputTarget_free(target_);

        ACaptureSessionOutputContainer_remove(outputContainer_, sessionOutput_);
        ACaptureSessionOutput_free(sessionOutput_);

        ANativeWindow_release(window_);
    }
    ACaptureSessionOutputContainer_free(outputContainer_);

    for (auto &cam: cameras_)
        if (cam.second.device_) ACameraDevice_close(cam.second.device_);
    cameras_.clear();
    if (cameraMgr_) {
        ACameraManager_unregisterAvailabilityCallback(cameraMgr_, GetManagerListener());
        ACameraManager_delete(cameraMgr_);
        cameraMgr_ = nullptr;
    }

    if (reader_) {
        delete reader_;
        reader_ = nullptr;
    }
    if (surface_) {
        env_->DeleteGlobalRef(surface_);
        surface_ = nullptr;
    }
}

const std::pair<int32_t, int32_t> &Camera::GetDimensions() const {
    return dimensions_;
}

void Camera::onPreviewSurfaceCreated(JNIEnv *env, jobject surface) {
    CreateSession(env, surface);
    StartPreview(true);
}

void Camera::CreateSession(JNIEnv *env, jobject surface) {
    surface_ = env_->NewGlobalRef(surface);
    ASSERT(reader_, "reader_ is NULL!")
    reader_->SetMirrorWindow(ANativeWindow_fromSurface(env, surface));
    window_ = reader_->GetNativeWindow();

    ACaptureSessionOutputContainer_create(&outputContainer_);
    ANativeWindow_acquire(window_);
    ACaptureSessionOutput_create(window_, &sessionOutput_);
    ACaptureSessionOutputContainer_add(outputContainer_, sessionOutput_);
    ACameraOutputTarget_create(window_, &target_);
    ACameraDevice_createCaptureRequest(cameras_[activeCameraId_].device_,
                                       TEMPLATE_PREVIEW, &request_);
    ACaptureRequest_addTarget(request_, target_);

    // Create a capture session for the given preview request
    captureSessionState_ = CaptureSessionState::READY;
    ACameraDevice_createCaptureSession(
            cameras_[activeCameraId_].device_, outputContainer_,
            GetSessionListener(), &captureSession_);
}

/**
 * Loop through cameras on the system, pick up
 * 1) back facing one if available
 * 2) otherwise pick the first one reported to us
 */
void Camera::EnumerateCamera() {
    ACameraIdList *cameraIds = nullptr;
    ACameraManager_getCameraIdList(cameraMgr_, &cameraIds);

    for (int i = 0; i < cameraIds->numCameras; ++i) {
        const char *id = cameraIds->cameraIds[i];

        ACameraMetadata *metadataObj;
        ACameraManager_getCameraCharacteristics(cameraMgr_, id, &metadataObj);

        int32_t count = 0;
        const uint32_t *tags = nullptr;
        ACameraMetadata_getAllTags(metadataObj, &count, &tags);
        for (int tagIdx = 0; tagIdx < count; ++tagIdx)
            if (ACAMERA_LENS_FACING == tags[tagIdx]) {
                ACameraMetadata_const_entry lensInfo = {0};
                ACameraMetadata_getConstEntry(metadataObj, tags[tagIdx], &lensInfo);
                CameraId cam(id);
                cam.facing_ = static_cast<acamera_metadata_enum_android_lens_facing_t>(
                        lensInfo.data.u8[0]);
                cam.device_ = nullptr;
                cameras_[cam.id_] = cam;
                if (cam.facing_ == ACAMERA_LENS_FACING_BACK)
                    activeCameraId_ = cam.id_;
                break;
            }
        ACameraMetadata_free(metadataObj);
    }

    ASSERT(!cameras_.empty(), "No Camera Available on the device")
    if (activeCameraId_.length() == 0) {
        // if no back facing camera found, pick up the first one to use...
        activeCameraId_ = cameras_.begin()->second.id_;
    }
    ACameraManager_deleteCameraIdList(cameraIds);
}

bool Camera::MatchCaptureSizeRequest(std::pair<int32_t, int32_t> *dimen) {
    ACameraMetadata *metadata;
    ACameraManager_getCameraCharacteristics(
            cameraMgr_, activeCameraId_.c_str(), &metadata);
    ACameraMetadata_const_entry entry;
    ACameraMetadata_getConstEntry(
            metadata, ACAMERA_SCALER_AVAILABLE_STREAM_CONFIGURATIONS, &entry);
    // format of the data: format, width, height, input?, type int32
    bool foundIt = false;
    std::pair<int32_t, int32_t> ultimate(720, 720);

    for (uint32_t i = 0; i < entry.count; i += 4) {
        int32_t input = entry.data.i32[i + 3];
        int32_t format = entry.data.i32[i + 0];
        if (input || format != VIS_IMAGE_FORMAT) continue;

        std::pair<int32_t, int32_t> ent(entry.data.i32[i + 1], entry.data.i32[i + 2]);
        if (dimen->first * ent.second != dimen->second * ent.first) continue; // is the same ratio
        if (ultimate.first >= ent.first & ultimate.second >= ent.second) {
            foundIt = true;
            ultimate = ent;
        }
    }
    if (!foundIt) LOGW("Did not find any compatible camera resolution, taking 720x720");

    *dimen = ultimate;
    return foundIt;
}

/**
 * GetSensorOrientation()
 *     Retrieve current sensor orientation regarding to the phone device
 * orientation
 *     SensorOrientation is NOT settable.
 */
/*int NDKCamera::GetCameraSensorOrientation(int32_t requestFacing) {
    if (!cameraMgr_) return false;
    ASSERT(requestFacing == ACAMERA_LENS_FACING_BACK, "Only support rear facing camera")
    int32_t facing, angle;

    ACameraMetadata *metadataObj;
    ACameraMetadata_const_entry face, orientation;
    ACameraManager_getCameraCharacteristics(
            cameraMgr_, activeCameraId_.c_str(), &metadataObj);
    ACameraMetadata_getConstEntry(metadataObj, ACAMERA_LENS_FACING, &face);
    facing = static_cast<int32_t>(face.data.u8[0]);

    ACameraMetadata_getConstEntry(
            metadataObj, ACAMERA_SENSOR_ORIENTATION, &orientation);
    ACameraMetadata_free(metadataObj);
    angle = orientation.data.i32[0];

    if (facing == requestFacing) return angle;
    ASSERT(false, "Failed for GetSensorOrientation()")
}*/

void Camera::StartPreview(bool start) {
    if (start)
        ACameraCaptureSession_setRepeatingRequest(
                captureSession_, nullptr, 1,
                &request_, nullptr);
    else if (captureSessionState_ == CaptureSessionState::ACTIVE)
        ACameraCaptureSession_stopRepeating(captureSession_);
}

bool Camera::SetRecording(bool b) {
    if (reader_)
        return reader_->SetRecording(b);
    else return false;
}

void Camera::onPreviewSurfaceDestroyed(JNIEnv *env, jobject surface) {
    jclass cls = env->FindClass("android/view/Surface");
    jmethodID toString =
            env->GetMethodID(cls, "toString", "()Ljava/lang/String;");

    auto destroyObjStr =
            reinterpret_cast<jstring>(env->CallObjectMethod(surface, toString));
    const char *destroyObjName = env->GetStringUTFChars(destroyObjStr, nullptr);

    auto appObjStr = reinterpret_cast<jstring>(
            env->CallObjectMethod(surface_, toString));
    const char *appObjName = env->GetStringUTFChars(appObjStr, nullptr);
    ASSERT(!strcmp(destroyObjName, appObjName), "object Name MisMatch")

    env->ReleaseStringUTFChars(destroyObjStr, destroyObjName);
    env->ReleaseStringUTFChars(appObjStr, appObjName);

    StartPreview(false);
}

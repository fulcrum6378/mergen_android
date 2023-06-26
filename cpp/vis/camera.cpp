#include <cstdlib>

#include "camera.h"
#include "../global.h"

Camera::Camera(Queuer *queuer) : captureSessionState_(CaptureSessionState::MAX_STATE) {
    cameraMgr_ = ACameraManager_create();
    ASSERT(cameraMgr_, "Failed to create cameraManager")
    cameras_.clear();
    EnumerateCamera();
    ASSERT(!activeCameraId_.empty(), "Unknown ActiveCameraIdx")

    DetermineCaptureDimensions(&dimensions_);
    reader_ = new ImageReader(&dimensions_, queuer);
    readerWindow_ = reader_->GetNativeWindow();

    cameraDeviceListener_ = {
            .context = this,
            .onDisconnected = [](void *ctx, ACameraDevice *dev) {
                auto *cam = reinterpret_cast<Camera *>(ctx);
                std::string id(ACameraDevice_getId(dev));
                LOGW("device %s is disconnected", id.c_str());

                ACameraDevice_close(cam->cameras_[id].device_);
                cam->cameras_.erase(id);
            },
            .onError = nullptr,
    };
    ACameraManager_openCamera(
            cameraMgr_, activeCameraId_.c_str(), &cameraDeviceListener_,
            &cameras_[activeCameraId_].device_);

    sessionListener = {
            .context = this,
            .onClosed = [](void *ctx, ACameraCaptureSession *ses) {
                reinterpret_cast<Camera *>(ctx)->captureSessionState_ = CaptureSessionState::CLOSED;
            },
            .onReady = [](void *ctx, ACameraCaptureSession *ses) {
                reinterpret_cast<Camera *>(ctx)->captureSessionState_ = CaptureSessionState::READY;
            },
            .onActive = [](void *ctx, ACameraCaptureSession *ses) {
                reinterpret_cast<Camera *>(ctx)->captureSessionState_ = CaptureSessionState::ACTIVE;
            },
    };

    // Initialize camera controls(exposure time and sensitivity), pick
    // up value of 2% * range + min as starting value (just a number, no magic)
    ACameraMetadata *metadataObj;
    ACameraManager_getCameraCharacteristics(
            cameraMgr_, activeCameraId_.c_str(), &metadataObj);
}

const std::pair<int32_t, int32_t> &Camera::GetDimensions() const {
    return dimensions_;
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

bool Camera::DetermineCaptureDimensions(std::pair<int32_t, int32_t> *dimen) {
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
        //if (ent.first == ent.second) continue; // discard square dimensions

        if (abs(ent.second - VIS_IMAGE_NEAREST_HEIGHT) <
            abs(ultimate.second - VIS_IMAGE_NEAREST_HEIGHT)) {
            foundIt = true;
            ultimate = ent;
        }
    }
    if (!foundIt) LOGW("Did not find any compatible camera resolution, taking 720x720");

    *dimen = ultimate;
    return foundIt;
}

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

/** The default FPS is in range 22..24! */
void Camera::CreateSession(ANativeWindow *displayWindow) {
    displayWindow_ = displayWindow;
    ACaptureSessionOutputContainer_create(&outputContainer_);

    // READER
    ANativeWindow_acquire(readerWindow_);
    ACaptureSessionOutput_create(readerWindow_, &readerOutput_);
    ACaptureSessionOutputContainer_add(outputContainer_, readerOutput_);
    ACameraOutputTarget_create(readerWindow_, &readerTarget_);
    ACameraDevice_createCaptureRequest(cameras_[activeCameraId_].device_,
                                       TEMPLATE_PREVIEW, &readerRequest_);
    ACaptureRequest_addTarget(readerRequest_, readerTarget_);

    // DISPLAY
    ANativeWindow_acquire(displayWindow_);
    ACaptureSessionOutput_create(displayWindow_, &displayOutput_);
    ACaptureSessionOutputContainer_add(outputContainer_, displayOutput_);
    ACameraOutputTarget_create(displayWindow_, &displayTarget_);
    ACameraDevice_createCaptureRequest(cameras_[activeCameraId_].device_,
                                       TEMPLATE_PREVIEW, &displayRequest_);
    ACaptureRequest_addTarget(displayRequest_, displayTarget_);

    captureSessionState_ = CaptureSessionState::READY;
    ACameraDevice_createCaptureSession(
            cameras_[activeCameraId_].device_, outputContainer_,
            &sessionListener, &captureSession_);
    Watch(true);
}

void Camera::Watch(bool start) {
    if (start)
        ACameraCaptureSession_setRepeatingRequest(
                captureSession_, nullptr, 2,
                new ACaptureRequest *[2]{readerRequest_, displayRequest_},
                nullptr);
    else if (captureSessionState_ == CaptureSessionState::ACTIVE)
        ACameraCaptureSession_stopRepeating(captureSession_);
}

bool Camera::SetRecording(bool b) {
    if (reader_)
        return reader_->SetRecording(b);
    else return false;
}

Camera::~Camera() {
    Watch(false);
    ACameraCaptureSession_close(captureSession_);

    if (readerWindow_) {
        ACaptureRequest_removeTarget(readerRequest_, readerTarget_);
        ACaptureRequest_free(readerRequest_);
        ACameraOutputTarget_free(readerTarget_);

        ACaptureSessionOutputContainer_remove(outputContainer_, readerOutput_);
        ACaptureSessionOutput_free(readerOutput_);

        ANativeWindow_release(readerWindow_);
    }
    if (displayWindow_) {
        ACaptureRequest_removeTarget(displayRequest_, displayTarget_);
        ACaptureRequest_free(displayRequest_);
        ACameraOutputTarget_free(displayTarget_);

        ACaptureSessionOutputContainer_remove(outputContainer_, displayOutput_);
        ACaptureSessionOutput_free(displayOutput_);

        ANativeWindow_release(displayWindow_);
    }
    ACaptureSessionOutputContainer_free(outputContainer_);

    for (auto &cam: cameras_)
        if (cam.second.device_) ACameraDevice_close(cam.second.device_);
    cameras_.clear();
    if (cameraMgr_) {
        ACameraManager_delete(cameraMgr_);
        cameraMgr_ = nullptr;
    }

    if (reader_) {
        delete reader_;
        reader_ = nullptr;
    }
}

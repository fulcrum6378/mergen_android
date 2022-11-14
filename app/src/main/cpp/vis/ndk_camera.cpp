#include <camera/NdkCameraManager.h>
#include <cinttypes>
#include <cstring>
#include <media/NdkImage.h>
#include <queue>
#include <unistd.h>
#include <utility>

#include "ndk_camera.h"
#include "utils.h"
#include "../native_debug.h"

NDKCamera::NDKCamera()
        : cameraMgr_(nullptr),
        // TODO cameraFacing_(ACAMERA_LENS_FACING_BACK),
        // cameraOrientation_(0),
          outputContainer_(nullptr),
          captureSessionState_(CaptureSessionState::MAX_STATE) {
    requests_.resize(CAPTURE_REQUEST_COUNT);
    memset(requests_.data(), 0, requests_.size() * sizeof(requests_[0]));
    cameras_.clear();
    cameraMgr_ = ACameraManager_create();
    ASSERT(cameraMgr_, "Failed to create cameraManager")

    // Pick up a back-facing camera to preview
    EnumerateCamera();
    ASSERT(!activeCameraId_.empty(), "Unknown ActiveCameraIdx")

    // Create back facing camera device
    CALL_MGR(openCamera(cameraMgr_, activeCameraId_.c_str(), GetDeviceListener(),
                        &cameras_[activeCameraId_].device_))

    CALL_MGR(registerAvailabilityCallback(cameraMgr_, GetManagerListener()))

    // Initialize camera controls(exposure time and sensitivity), pick
    // up value of 2% * range + min as starting value (just a number, no magic)
    ACameraMetadata *metadataObj;
    CALL_MGR(getCameraCharacteristics(cameraMgr_, activeCameraId_.c_str(),
                                      &metadataObj))
}

void NDKCamera::CreateSession(ANativeWindow *previewWindow,
                              ANativeWindow *jpgWindow, bool manualPreview,
                              int32_t imageRotation) {
    // Create output from this app's ANativeWindow, and add into output container
    requests_[PREVIEW_REQUEST_IDX].outputNativeWindow_ = previewWindow;
    requests_[PREVIEW_REQUEST_IDX].template_ = TEMPLATE_PREVIEW;
    requests_[JPG_CAPTURE_REQUEST_IDX].outputNativeWindow_ = jpgWindow;
    requests_[JPG_CAPTURE_REQUEST_IDX].template_ = TEMPLATE_STILL_CAPTURE;

    CALL_CONTAINER(create(&outputContainer_))
    for (auto &req: requests_) {
        if (!req.outputNativeWindow_) continue;

        ANativeWindow_acquire(req.outputNativeWindow_);
        CALL_OUTPUT(create(req.outputNativeWindow_, &req.sessionOutput_))
        CALL_CONTAINER(add(outputContainer_, req.sessionOutput_))
        CALL_TARGET(create(req.outputNativeWindow_, &req.target_))
        CALL_DEV(createCaptureRequest(cameras_[activeCameraId_].device_,
                                      req.template_, &req.request_))
        CALL_REQUEST(addTarget(req.request_, req.target_))
    }

    // Create a capture session for the given preview request
    captureSessionState_ = CaptureSessionState::READY;
    CALL_DEV(createCaptureSession(cameras_[activeCameraId_].device_,
                                  outputContainer_, GetSessionListener(),
                                  &captureSession_))

    if (jpgWindow) {
        ACaptureRequest_setEntry_i32(requests_[JPG_CAPTURE_REQUEST_IDX].request_,
                                     ACAMERA_JPEG_ORIENTATION, 1, &imageRotation);
    }

    if (!manualPreview) {
        return;
    }
    /*
     * Only preview request is in manual mode, JPG is always in Auto mode
     * JPG capture mode could also be switch into manual mode and control
     * the capture parameters, this sample leaves JPG capture to be auto mode
     * (auto control has better effect than author's manual control)
     */
    uint8_t aeModeOff = ACAMERA_CONTROL_AE_MODE_OFF;
    CALL_REQUEST(setEntry_u8(requests_[PREVIEW_REQUEST_IDX].request_,
                             ACAMERA_CONTROL_AE_MODE, 1, &aeModeOff))
}

void NDKCamera::CreateSession(ANativeWindow *previewWindow) {
    CreateSession(previewWindow, nullptr, false, 0);
}

NDKCamera::~NDKCamera() {
    // stop session if it is on:
    if (captureSessionState_ == CaptureSessionState::ACTIVE) {
        ACameraCaptureSession_stopRepeating(captureSession_);
    }
    ACameraCaptureSession_close(captureSession_);

    for (auto &req: requests_) {
        if (!req.outputNativeWindow_) continue;

        CALL_REQUEST(removeTarget(req.request_, req.target_))
        ACaptureRequest_free(req.request_);
        ACameraOutputTarget_free(req.target_);

        CALL_CONTAINER(remove(outputContainer_, req.sessionOutput_))
        ACaptureSessionOutput_free(req.sessionOutput_);

        ANativeWindow_release(req.outputNativeWindow_);
    }
    requests_.resize(0);
    ACaptureSessionOutputContainer_free(outputContainer_);

    for (auto &cam: cameras_) {
        if (cam.second.device_) CALL_DEV(close(cam.second.device_))
    }
    cameras_.clear();
    if (cameraMgr_) {
        CALL_MGR(unregisterAvailabilityCallback(cameraMgr_, GetManagerListener()))
        ACameraManager_delete(cameraMgr_);
        cameraMgr_ = nullptr;
    }
}

/**
 * EnumerateCamera()
 *     Loop through cameras on the system, pick up
 *     1) back facing one if available
 *     2) otherwise pick the first one reported to us
 */
void NDKCamera::EnumerateCamera() {
    ACameraIdList *cameraIds = nullptr;
    CALL_MGR(getCameraIdList(cameraMgr_, &cameraIds))

    for (int i = 0; i < cameraIds->numCameras; ++i) {
        const char *id = cameraIds->cameraIds[i];

        ACameraMetadata *metadataObj;
        CALL_MGR(getCameraCharacteristics(cameraMgr_, id, &metadataObj))

        int32_t count = 0;
        const uint32_t *tags = nullptr;
        ACameraMetadata_getAllTags(metadataObj, &count, &tags);
        for (int tagIdx = 0; tagIdx < count; ++tagIdx) {
            if (ACAMERA_LENS_FACING == tags[tagIdx]) {
                ACameraMetadata_const_entry lensInfo = {
                        0,
                };
                CALL_METADATA(getConstEntry(metadataObj, tags[tagIdx], &lensInfo))
                CameraId cam(id);
                cam.facing_ = static_cast<acamera_metadata_enum_android_lens_facing_t>(
                        lensInfo.data.u8[0]);
                cam.device_ = nullptr;
                cameras_[cam.id_] = cam;
                if (cam.facing_ == ACAMERA_LENS_FACING_BACK) {
                    activeCameraId_ = cam.id_;
                }
                break;
            }
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

/**
 * GetSensorOrientation()
 *     Retrieve current sensor orientation regarding to the phone device
 * orientation
 *     SensorOrientation is NOT settable.
 */
/*bool NDKCamera::GetSensorOrientation(int32_t *facing, int32_t *angle) {
    if (!cameraMgr_) return false;

    ACameraMetadata *metadataObj;
    ACameraMetadata_const_entry face, orientation;
    CALL_MGR(getCameraCharacteristics(
            cameraMgr_, activeCameraId_.c_str(), &metadataObj))
    CALL_METADATA(getConstEntry(metadataObj, ACAMERA_LENS_FACING, &face))
    cameraFacing_ = static_cast<int32_t>(face.data.u8[0]);

    CALL_METADATA(
            getConstEntry(metadataObj, ACAMERA_SENSOR_ORIENTATION, &orientation))

    LOGI("====Current SENSOR_ORIENTATION: %8d", orientation.data.i32[0]);

    ACameraMetadata_free(metadataObj);
    cameraOrientation_ = orientation.data.i32[0];

    if (facing) *facing = cameraFacing_;
    if (angle) *angle = cameraOrientation_;
    return true;
}*/

/** Toggle preview start/stop */
void NDKCamera::StartPreview(bool start) {
    if (start) CALL_SESSION(setRepeatingRequest(
            captureSession_, nullptr, 1,
            &requests_[PREVIEW_REQUEST_IDX].request_, nullptr))
    else if (captureSessionState_ == CaptureSessionState::ACTIVE)
        ACameraCaptureSession_stopRepeating(captureSession_);
}

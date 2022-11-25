#include <camera/NdkCameraManager.h>
#include <cinttypes>
#include <cstring>
#include <media/NdkImage.h>
#include <queue>
#include <unistd.h>
#include <utility>

#include "ndk_camera.h"
#include "utils.h"
#include "../otr/debug.h"

NDKCamera::NDKCamera()
        : cameraMgr_(nullptr),
        // cameraFacing_(ACAMERA_LENS_FACING_BACK),
        // cameraOrientation_(0),
          window_(nullptr),
          sessionOutput_(nullptr),
          target_(nullptr),
          request_(nullptr),
          reader(nullptr),
          outputContainer_(nullptr),
          captureSessionState_(CaptureSessionState::MAX_STATE) {
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

/**
 * A helper class to assist image size comparison, by comparing the absolute size
 * regardless of the portrait or landscape mode.
 */
class DisplayDimension {
public:
    DisplayDimension(int32_t w, int32_t h) : w_(w), h_(h), portrait_(false) {
        if (h > w) {
            // make it landscape
            w_ = h;
            h_ = w;
            portrait_ = true;
        }
    }

    DisplayDimension(const DisplayDimension &other) {
        w_ = other.w_;
        h_ = other.h_;
        portrait_ = other.portrait_;
    }

    DisplayDimension() {
        w_ = 0;
        h_ = 0;
        portrait_ = false;
    }

    DisplayDimension &operator=(const DisplayDimension &other) = default;

    bool IsSameRatio(DisplayDimension &other) const {
        return (w_ * other.h_ == h_ * other.w_);
    }

    bool operator>(DisplayDimension &other) const {
        return (w_ >= other.w_ & h_ >= other.h_);
    }

    bool operator==(DisplayDimension &other) const {
        return (w_ == other.w_ && h_ == other.h_ && portrait_ == other.portrait_);
    }

    DisplayDimension operator-(DisplayDimension &other) const {
        DisplayDimension delta(w_ - other.w_, h_ - other.h_);
        return delta;
    }

    //void Flip() { portrait_ = !portrait_; }

    bool IsPortrait() const { return portrait_; }

    //int32_t width() const { return w_; }

    //int32_t height() const { return h_; }

    int32_t org_width() { return (portrait_ ? h_ : w_); }

    int32_t org_height() { return (portrait_ ? w_ : h_); }

private:
    int32_t w_, h_;
    bool portrait_;
};

bool NDKCamera::MatchCaptureSizeRequest(
        int32_t requestWidth, int32_t requestHeight, ImageFormat *resView) {
    DisplayDimension disp(requestWidth, requestHeight);
    //if (cameraOrientation_ == 90 || cameraOrientation_ == 270) disp.Flip();
    ACameraMetadata *metadata;
    CALL_MGR(getCameraCharacteristics(
            cameraMgr_, activeCameraId_.c_str(), &metadata))
    ACameraMetadata_const_entry entry;
    CALL_METADATA(getConstEntry(
            metadata, ACAMERA_SCALER_AVAILABLE_STREAM_CONFIGURATIONS, &entry))
    // format of the data: format, width, height, input?, type int32
    bool foundIt = false;
    DisplayDimension foundRes(4000, 4000);

    for (uint32_t i = 0; i < entry.count; i += 4) {
        int32_t input = entry.data.i32[i + 3];
        int32_t format = entry.data.i32[i + 0];
        if (input) continue;

        if (format == AIMAGE_FORMAT_YUV_420_888) {
            DisplayDimension res(entry.data.i32[i + 1], entry.data.i32[i + 2]);
            if (!disp.IsSameRatio(res)) continue;
            if (foundRes > res) {
                foundIt = true;
                foundRes = res;
            }
        }
    }

    if (foundIt) {
        resView->width = foundRes.org_width();
        resView->height = foundRes.org_height();
    } else {
        LOGW("Did not find any compatible camera resolution, taking 640x480");
        if (disp.IsPortrait()) {
            resView->width = 480;
            resView->height = 640;
        } else {
            resView->width = 640;
            resView->height = 480;
        }
    }
    resView->format = AIMAGE_FORMAT_YUV_420_888;
    return foundIt;
}

void NDKCamera::CreateSession(ANativeWindow *previewWindow, int32_t /*imageRotation*/) {
    // Create output from this app's ANativeWindow, and add into output container
    //window = previewWindow;
    // FIXME needs Android 30 ANativeWindow_setFrameRate(outputNativeWindow_,
    //    10, ANATIVEWINDOW_FRAME_RATE_COMPATIBILITY_FIXED_SOURCE);

    ImageFormat view{0, 0, AIMAGE_FORMAT_YUV_420_888};
    MatchCaptureSizeRequest(ANativeWindow_getWidth(previewWindow),
                            ANativeWindow_getHeight(previewWindow), &view);
    ASSERT(view.width && view.height, "Could not find supportable resolution")

    reader = new ImageReader(&view); // reader_->SetPresentRotation(0);
    reader->RegisterCallback(
            this, [/*this*/](void *ctx, const char *str) -> void {
                //reinterpret_cast<CameraEngine* >(ctx)->OnPhotoTaken(str);
                /*JNIEnv *jni;
  app_->activity->vm->AttachCurrentThread(&jni, NULL);

  // Default class retrieval
  jclass clazz = jni->GetObjectClass(app_->activity->clazz);
  jmethodID methodID = jni->GetMethodID(clazz, "OnPhotoTaken", "(Ljava/lang/String;)V");
  jstring javaName = jni->NewStringUTF(fileName);

  jni->CallVoidMethod(app_->activity->clazz, methodID, javaName);
  app_->activity->vm->DetachCurrentThread();*/
            });
    window_ = reader->GetNativeWindow();

    CALL_CONTAINER(create(&outputContainer_))
    if (window_) {
        ANativeWindow_acquire(window_);
        CALL_OUTPUT(create(window_, &sessionOutput_))
        CALL_CONTAINER(add(outputContainer_, sessionOutput_))
        CALL_TARGET(create(window_, &target_))
        CALL_DEV(createCaptureRequest(cameras_[activeCameraId_].device_,
                                      TEMPLATE_PREVIEW, &request_))
        CALL_REQUEST(addTarget(request_, target_))
    }

    // Create a capture session for the given preview request
    captureSessionState_ = CaptureSessionState::READY;
    CALL_DEV(createCaptureSession(cameras_[activeCameraId_].device_,
                                  outputContainer_, GetSessionListener(),
                                  &captureSession_))
}

NDKCamera::~NDKCamera() {
    // stop session if it is on:
    if (captureSessionState_ == CaptureSessionState::ACTIVE)
        ACameraCaptureSession_stopRepeating(captureSession_);
    ACameraCaptureSession_close(captureSession_);

    if (window_) {
        CALL_REQUEST(removeTarget(request_, target_))
        ACaptureRequest_free(request_);
        ACameraOutputTarget_free(target_);

        CALL_CONTAINER(remove(outputContainer_, sessionOutput_))
        ACaptureSessionOutput_free(sessionOutput_);

        ANativeWindow_release(window_);
    }
    ACaptureSessionOutputContainer_free(outputContainer_);

    for (auto &cam: cameras_)
        if (cam.second.device_) CALL_DEV(close(cam.second.device_))
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
        for (int tagIdx = 0; tagIdx < count; ++tagIdx)
            if (ACAMERA_LENS_FACING == tags[tagIdx]) {
                ACameraMetadata_const_entry lensInfo = {0};
                CALL_METADATA(getConstEntry(metadataObj, tags[tagIdx], &lensInfo))
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

// Toggle preview start/stop
void NDKCamera::StartPreview(bool start) {
    if (start) CALL_SESSION(setRepeatingRequest(
            captureSession_, GetCaptureCallback(), 1,
            &request_, nullptr))
    else if (captureSessionState_ == CaptureSessionState::ACTIVE)
        ACameraCaptureSession_stopRepeating(captureSession_);
}

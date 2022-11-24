#ifndef VIS_NDK_CAMERA_H
#define VIS_NDK_CAMERA_H

#include <camera/NdkCameraDevice.h>
#include <camera/NdkCameraError.h>
#include <camera/NdkCameraManager.h>
#include <camera/NdkCameraMetadataTags.h>
#include <map>
#include <string>
#include <vector>

#include "image_reader.h"

enum class CaptureSessionState : int32_t {
    READY = 0,  // session is ready
    ACTIVE,     // session is busy
    CLOSED,     // session is closed(by itself or a new session evicts)
    MAX_STATE
};

class CameraId;

class NDKCamera {
private:
    ACameraManager *cameraMgr_;
    std::map<std::string, CameraId> cameras_;
    std::string activeCameraId_;

    ANativeWindow *outputNativeWindow_{};
    ACaptureSessionOutput *sessionOutput_{};
    ACameraOutputTarget *target_{};
    ACaptureRequest *request_{};
    ImageReader *reader_{};

    ACaptureSessionOutputContainer *outputContainer_;
    ACameraCaptureSession *captureSession_{};
    CaptureSessionState captureSessionState_;

    ACameraManager_AvailabilityCallbacks *GetManagerListener();

    ACameraDevice_stateCallbacks *GetDeviceListener();

    ACameraCaptureSession_stateCallbacks *GetSessionListener();

    ACameraCaptureSession_captureCallbacks *GetCaptureCallback();

public:
    NDKCamera();

    ~NDKCamera();

    void EnumerateCamera(void);

    bool MatchCaptureSizeRequest(int32_t requestWidth, int32_t requestHeight, ImageFormat *view);

    void CreateSession(ANativeWindow *previewWindow, int32_t imageRotation);

    //bool GetSensorOrientation(int32_t *facing, int32_t *angle);

    void OnDeviceState(ACameraDevice *dev);

    void OnDeviceError(ACameraDevice *dev, int err);

    void OnSessionState(ACameraCaptureSession *ses, CaptureSessionState state);

    void StartPreview(bool start);
};

// Helper classes to hold enumerated camera
class CameraId {
public:
    ACameraDevice *device_;
    std::string id_;
    acamera_metadata_enum_android_lens_facing_t facing_;

    explicit CameraId(const char *id) :
            device_(nullptr), facing_(ACAMERA_LENS_FACING_FRONT) {
        id_ = id;
    }

    explicit CameraId(void) { CameraId(""); }
};

#endif //VIS_NDK_CAMERA_H

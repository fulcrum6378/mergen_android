#ifndef VIS_NDK_CAMERA_H
#define VIS_NDK_CAMERA_H

#include <map>
#include <media/NdkImage.h>
#include <string>

#include "utils.h"

#define VIS_IMAGE_FORMAT AIMAGE_FORMAT_YUV_420_888
// together with AIMAGE_FORMAT_JPEG, these are the only supported options for my phone apparently!

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

    ANativeWindow *window_{};
    ACaptureSessionOutput *sessionOutput_{};
    ACameraOutputTarget *target_{};
    ACaptureRequest *request_{};

    ACaptureSessionOutputContainer *outputContainer_;
    ACameraCaptureSession *captureSession_{};
    CaptureSessionState captureSessionState_;

    ACameraManager_AvailabilityCallbacks *GetManagerListener();

    ACameraDevice_stateCallbacks *GetDeviceListener();

    ACameraCaptureSession_stateCallbacks *GetSessionListener();

public:
    NDKCamera();

    ~NDKCamera();

    void EnumerateCamera(void);

    bool MatchCaptureSizeRequest(int32_t requestWidth, int32_t requestHeight, ImageFormat *view);

    void CreateSession(ANativeWindow *window);

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

    explicit CameraId(const char *id) : device_(nullptr), facing_(ACAMERA_LENS_FACING_FRONT) {
        id_ = id;
    }

    explicit CameraId(void) { CameraId(""); }
};

#endif //VIS_NDK_CAMERA_H

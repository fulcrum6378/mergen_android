#ifndef VIS_NDK_CAMERA_H
#define VIS_NDK_CAMERA_H

#include <camera/NdkCameraDevice.h>
#include <camera/NdkCameraManager.h>
#include <camera/NdkCameraMetadataTags.h>
#include <map>
#include <string>
#include <utility>

#include "../global.h"

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

    ANativeWindow *window_;
    ACaptureSessionOutput *sessionOutput_;
    ACameraOutputTarget *target_;
    ACaptureRequest *request_;

    ACaptureSessionOutputContainer *outputContainer_;
    ACameraCaptureSession *captureSession_;
    CaptureSessionState captureSessionState_;

    // Private Listeners
    ACameraManager_AvailabilityCallbacks *GetManagerListener();

    ACameraDevice_stateCallbacks *GetDeviceListener();

    ACameraCaptureSession_stateCallbacks *GetSessionListener();

public:
    NDKCamera();

    ~NDKCamera();

    void EnumerateCamera(void);

    bool MatchCaptureSizeRequest(std::pair<int32_t, int32_t> *dimen);

    void CreateSession(ANativeWindow *window);

    //bool GetSensorOrientation(int32_t *facing, int32_t *angle);

    void StartPreview(bool start);

    // Public Listeners
    void OnDeviceState(ACameraDevice *dev);

    void OnDeviceError(ACameraDevice *dev, int err);

    void OnSessionState(ACameraCaptureSession *ses, CaptureSessionState state);
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

#ifndef VIS_CAMERA_H
#define VIS_CAMERA_H

#include <camera/NdkCameraDevice.h>
#include <camera/NdkCameraManager.h>
#include <camera/NdkCameraMetadataTags.h>
#include <map>
#include <string>
#include <utility>

#include "image_reader.h"

enum class CaptureSessionState : int32_t {
    READY,      // session is ready
    ACTIVE,     // session is busy
    CLOSED,     // session is closed(by itself or a new session evicts)
    MAX_STATE
};

class CameraId;

class Camera {
private:
    ImageReader *reader_{};
    std::pair<int32_t, int32_t> dimensions_;

    // Managing cameras
    ACameraManager *cameraMgr_;
    std::map<std::string, CameraId> cameras_;
    std::string activeCameraId_;
    ACameraDevice_stateCallbacks cameraDeviceListener_{};

    // Capture session (GLOBAL)
    ACaptureSessionOutputContainer *outputContainer_{};
    ACameraCaptureSession *captureSession_{};
    CaptureSessionState captureSessionState_;
    ACameraCaptureSession_stateCallbacks sessionListener{};

    // Capture session (Reader)
    ANativeWindow *readerWindow_{};
    ACaptureSessionOutput *readerOutput_{};
    ACameraOutputTarget *readerTarget_{};
    ACaptureRequest *readerRequest_{};

    // Capture session (Display)
    ANativeWindow *displayWindow_{};
    ACaptureSessionOutput *displayOutput_{};
    ACameraOutputTarget *displayTarget_{};
    ACaptureRequest *displayRequest_{};


    void EnumerateCamera(void);

    bool DetermineCaptureDimensions(std::pair<int32_t, int32_t> *dimen);

    //int32_t GetCameraSensorOrientation(int32_t facing);

    void Watch(bool start);

public:
    Camera(Queuer *queuer);

    const std::pair<int32_t, int32_t> &GetDimensions() const;

    void CreateSession(ANativeWindow *displayWindow);

    bool SetRecording(bool b);

    ~Camera();
};

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

#endif //VIS_CAMERA_H

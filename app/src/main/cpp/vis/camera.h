#ifndef VIS_CAMERA_H
#define VIS_CAMERA_H

#include <camera/NdkCameraDevice.h>
#include <camera/NdkCameraManager.h>
#include <camera/NdkCameraMetadataTags.h>
#include <map>
#include <string>
#include <utility>

#include "image_reader.h"
#include "../global.h"

enum class CaptureSessionState : int32_t {
    READY = 0,  // session is ready
    ACTIVE,     // session is busy
    CLOSED,     // session is closed(by itself or a new session evicts)
    MAX_STATE
};

class CameraId;

class Camera {
private:
    JNIEnv *env_;
    jobject surface_;
    ImageReader *reader_{};
    std::pair<int32_t, int32_t> dimensions_;

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

    void CreateSession(JNIEnv *env, jobject surface);

    void EnumerateCamera(void);

    bool MatchCaptureSizeRequest(std::pair<int32_t, int32_t> *dimen);

    void StartPreview(bool start);


    // listeners.cpp
    ACameraManager_AvailabilityCallbacks *GetManagerListener();

    ACameraDevice_stateCallbacks *GetDeviceListener();

    ACameraCaptureSession_stateCallbacks *GetSessionListener();

public:
    Camera(JNIEnv *env, jint w, jint h);

    ~Camera();

    const std::pair<int32_t, int32_t> &GetDimensions() const;

    void onPreviewSurfaceCreated(JNIEnv *env, jobject surface);

    //int32_t GetCameraSensorOrientation(int32_t facing);

    bool SetRecording(bool b);

    void onPreviewSurfaceDestroyed(JNIEnv *env, jobject surface);


    // listeners.cpp
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

#endif //VIS_CAMERA_H

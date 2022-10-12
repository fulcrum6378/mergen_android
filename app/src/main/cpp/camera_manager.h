#ifndef CAMERA_NATIVE_CAMERA_H
#define CAMERA_NATIVE_CAMERA_H

#include <string>
#include <vector>
#include <map>
#include <camera/NdkCameraManager.h>
#include <camera/NdkCameraError.h>
#include <camera/NdkCameraDevice.h>
#include <camera/NdkCameraMetadataTags.h>

enum class CaptureSessionState : int32_t {
    READY = 0,  // session is ready
    ACTIVE,     // session is busy
    CLOSED,     // session is closed(by itself or a new session evicts)
    MAX_STATE
};

/*
 * ImageFormat:
 *     A Data Structure to communicate resolution between camera and ImageReader
 */
struct ImageFormat {
    int32_t width;
    int32_t height;

    int32_t format;  // FIXME MAHDI: Through out this demo, the format is fixed to YUV_420 format
};

enum PREVIEW_INDICES {
    PREVIEW_REQUEST_IDX = 0,
    JPG_CAPTURE_REQUEST_IDX,
    CAPTURE_REQUEST_COUNT,
};

struct CaptureRequestInfo {
    ANativeWindow *outputNativeWindow_;
    ACaptureSessionOutput *sessionOutput_;
    ACameraOutputTarget *target_;
    ACaptureRequest *request_;
    ACameraDevice_request_template template_;
    int sessionSequenceId_;
};

class CameraId;

class NDKCamera {
private:
    ACameraManager *cameraMgr_;
    std::map<std::string, CameraId> cameras_;
    std::string activeCameraId_;
    uint32_t cameraFacing_;
    uint32_t cameraOrientation_;

    std::vector<CaptureRequestInfo> requests_;

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

    bool MatchCaptureSizeRequest(int32_t requestWidth, int32_t requestHeight,
                                 ImageFormat *view);

    bool MatchCaptureSizeRequest(int32_t requestWidth, int32_t requestHeight,
                                 ImageFormat *view, ImageFormat *capture);

    void CreateSession(ANativeWindow *previewWindow, ANativeWindow *jpgWindow,
                       bool manaulPreview, int32_t imageRotation);

    void CreateSession(ANativeWindow *previewWindow);

    bool GetSensorOrientation(int32_t *facing, int32_t *angle);

    void OnDeviceState(ACameraDevice *dev);

    void OnDeviceError(ACameraDevice *dev, int err);

    void OnSessionState(ACameraCaptureSession *ses, CaptureSessionState state);

    void OnCaptureSequenceEnd(ACameraCaptureSession *session, int sequenceId,
                              int64_t frameNumber);

    void OnCaptureFailed(ACameraCaptureSession *session, ACaptureRequest *request,
                         ACameraCaptureFailure *failure);

    void StartPreview(bool start);

    bool TakePhoto(void);

    void UpdateCameraRequestParameter(int32_t code, int64_t val);
};

// helper classes to hold enumerated camera
class CameraId {
public:
    ACameraDevice *device_;
    std::string id_;
    acamera_metadata_enum_android_lens_facing_t facing_;
    bool available_;  // free to use ( no other apps are using
    bool owner_;      // we are the owner of the camera
    explicit CameraId(const char *id)
            : device_(nullptr),
              facing_(ACAMERA_LENS_FACING_FRONT),
              available_(false),
              owner_(false) {
        id_ = id;
    }

    explicit CameraId(void) { CameraId(""); }
};

#endif

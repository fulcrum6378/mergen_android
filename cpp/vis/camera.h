#ifndef VIS_CAMERA_H
#define VIS_CAMERA_H

#include <camera/NdkCameraDevice.h>
#include <camera/NdkCameraManager.h>
#include <camera/NdkCameraMetadataTags.h>
#include <map>
#include <media/NdkImage.h>
#include <media/NdkImageReader.h>
#include <string>
#include <utility>

#include "bitmap_stream.h"

// together with AIMAGE_FORMAT_JPEG, these are the only supported options for my phone!
#define VIS_IMAGE_FORMAT AIMAGE_FORMAT_YUV_420_888
/** Let's make it a sqaure for less trouble, at least for now!
 * it would result in 2336x1080 in Galaxy A50. */
#define VIS_IMAGE_NEAREST_HEIGHT 1088
#define MAX_BUF_COUNT 4 // max image buffers

static bool VIS_SAVE = false;

enum class CaptureSessionState : int32_t {
    READY,      // session is ready
    ACTIVE,     // session is busy
    CLOSED,     // session is closed(by itself or a new session evicts)
    MAX_STATE
};

class CameraId;

class Camera {
private:
    /**
     * AImageReader creates an IGraphicBufferProducer (input) and an IGraphicBufferConsumer (output),
     * then it creates a BufferQueue from them, then it listens data from that IGraphicBufferConsumer,
     * THEN IT CREATES A Surface OUT OF THE IGraphicBufferProducer (input), which is the same as
     * ANativeWindow; so AImageReader has no choice except creating it, itself!
     */
    AImageReader *reader_{};
    std::pair<int32_t, int32_t> dimensions_;
    bool recording_{false};
    BitmapStream *bmp_stream_{};

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

    void Preview(bool start);

    void ImageCallback(AImageReader *reader);

public:
    Camera();

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

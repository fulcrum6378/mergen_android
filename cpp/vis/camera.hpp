#ifndef VIS_CAMERA_H
#define VIS_CAMERA_H

#include <camera/NdkCameraDevice.h>
#include <camera/NdkCameraManager.h>
#include <camera/NdkCameraMetadataTags.h>
#include <jni.h>
#include <map>
#include <media/NdkImageReader.h>
#include <string>
#include <utility>

#include "bitmap_stream.hpp"
#include "segmentation.hpp"

/** Together with AIMAGE_FORMAT_JPEG, these are the only supported options for my phone!
 * @see <a href="https://developer.android.com/reference/android/graphics/ImageFormat#YUV_420_888">
 * YUV_420_888</a> */
#define IMAGE_FORMAT AIMAGE_FORMAT_YUV_420_888
/** Let's make it a sqaure for less trouble, at least for now!
 * it would result in 2336x1080 in Galaxy A50. */
#define IMAGE_NEAREST_HEIGHT 1088
// maximum image buffers
#define MAX_BUF_COUNT 4

static bool VIS_SAVE = false;

enum class CaptureSessionState : int8_t {
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
    bool recording_{false};
    BitmapStream *bmp_stream_{};
    JavaVM *jvm_;
    jobject main_;
    jmethodID jmSignal_;
    int8_t debugMode_{0};

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

    // Image Analysis
    Segmentation *segmentation_;


    void EnumerateCamera();

    void DetermineCaptureDimensions();

    //int32_t GetCameraSensorOrientation(int8_t facing);

    void Preview(bool start);

    void ImageCallback(AImageReader *reader);

public:
    std::pair<int16_t, int16_t> dimensions;

    Camera(JavaVM *jvm, jobject main);

    void CreateSession(ANativeWindow *displayWindow);

    bool SetRecording(bool b, int8_t debugMode);

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

    explicit CameraId() : CameraId("") {}
};

#endif //VIS_CAMERA_H

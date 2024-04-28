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

/** Together with AIMAGE_FORMAT_JPEG, these are the only supported options for my phone!
 * @see <a href="https://developer.android.com/reference/android/graphics/ImageFormat#YUV_420_888">
 * YUV_420_888</a> */
#define IMAGE_FORMAT AIMAGE_FORMAT_YUV_420_888
// maximum image buffers
#define MAX_BUF_COUNT 4
// image processing method: 0 => BitmapStream, 1 => Segmentation, 2 => EdgeDetection.
#define VIS_METHOD 2

#if VIS_METHOD == 0

#include "bitmap.hpp"

#elif VIS_METHOD == 1

#include "segmentation_a.hpp"

#elif VIS_METHOD == 2

#include "segmentation_b.hpp"

#endif

enum class CaptureSessionState : int8_t {
    READY,      // session is ready
    ACTIVE,     // session is busy
    CLOSED,     // session is closed (by itself or a new session evicts)
    MAX_STATE
};

class CameraId;

class Camera {
public:
    Camera(JavaVM *jvm
#if VIS_METHOD == 1
            , jobject main
#elif VIS_METHOD == 2
            , jobject assets
#endif
    );

    void CreateSession(ANativeWindow *displayWindow);

    bool SetRecording(bool b);

    ~Camera();


    std::pair<int16_t, int16_t> dimensions;
#if VIS_METHOD == 0
    BitmapStream *bmp_stream_{};
#else
    Segmentation *segmentation;
#endif

private:
    void EnumerateCamera();

    void DetermineCaptureDimensions();

    //int32_t GetCameraSensorOrientation(int8_t facing);

    void Preview(bool start);

    void ImageCallback(AImageReader *reader);


    /**
     * AImageReader creates an IGraphicBufferProducer (input) and an IGraphicBufferConsumer (output),
     * then it creates a BufferQueue from them, then it listens data from that IGraphicBufferConsumer,
     * THEN IT CREATES A Surface OUT OF THE IGraphicBufferProducer (input), which is the same as
     * ANativeWindow; so AImageReader has no choice except creating it, itself!
     */
    AImageReader *reader_{};
    bool recording_{false};
    JavaVM *jvm_;
#if VIS_METHOD == 1
    jobject main_;
    jmethodID jmSignal_;
#endif

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
};

class CameraId {
public:
    explicit CameraId(const char *id) : device_(nullptr), facing_(ACAMERA_LENS_FACING_FRONT) {
        id_ = id;
    }

    explicit CameraId() : CameraId("") {}


    ACameraDevice *device_;
    std::string id_;
    acamera_metadata_enum_android_lens_facing_t facing_;
};

#endif //VIS_CAMERA_H

#ifndef AUD_CAMERA_ENGINE
#define AUD_CAMERA_ENGINE

#include <android/native_activity.h>
#include <android/native_window_jni.h>
#include <android/native_window.h>

#include "ndk_camera.h"

class CameraEngine {
public:
    explicit CameraEngine(JNIEnv *env);

    ~CameraEngine();

    // Manage NDKCamera Object
    void CreateCameraSession(jobject surface);

    void StartPreview(bool start);

    const ImageFormat &GetCompatibleCameraRes() const;

    //int32_t GetCameraSensorOrientation(int32_t facing);

    jobject GetSurfaceObject();

private:
    JNIEnv *env_;
    jobject surface_;
    NDKCamera *camera_;
    ImageFormat compatibleCameraRes_{};
};

#endif

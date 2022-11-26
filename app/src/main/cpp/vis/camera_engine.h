#ifndef VIS_CAMERA_ENGINE_H
#define VIS_CAMERA_ENGINE_H

#include "ndk_camera.h"

class CameraEngine {
public:
    explicit CameraEngine(JNIEnv *env, jint w, jint h);

    ~CameraEngine();

    void CreateCameraSession(JNIEnv *env, jobject surface);

    void StartPreview(bool start);

    const ImageFormat &GetCompatibleCameraRes() const;

    //int32_t GetCameraSensorOrientation(int32_t facing);

    jobject GetSurfaceObject();

private:
    JNIEnv *env_;
    jobject surface_;
    NDKCamera *camera_;
    ImageReader *reader_{};
    ImageFormat imageFormat{};
};

#endif //VIS_CAMERA_ENGINE_H

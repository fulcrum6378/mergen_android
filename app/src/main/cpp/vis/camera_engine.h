#ifndef VIS_CAMERA_ENGINE_H
#define VIS_CAMERA_ENGINE_H

#include "image_reader.h"
#include "ndk_camera.h"

class CameraEngine {
public:
    explicit CameraEngine(JNIEnv *env, jint w, jint h);

    ~CameraEngine();

    void CreateCameraSession(JNIEnv *env, jobject surface);

    void StartPreview(bool start);

    bool SetRecording(bool b);

    const std::pair<int32_t, int32_t> &GetDimensions() const;

    //int32_t GetCameraSensorOrientation(int32_t facing);

    jobject GetSurfaceObject();

private:
    JNIEnv *env_;
    jobject surface_;
    NDKCamera *camera_;
    ImageReader *reader_{};
    std::pair<int32_t, int32_t> dimensions_;
};

#endif //VIS_CAMERA_ENGINE_H

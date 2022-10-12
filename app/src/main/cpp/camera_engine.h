#ifndef __CAMERA_ENGINE_H__
#define __CAMERA_ENGINE_H__

#include <jni.h>
#include <android/native_activity.h>
#include <android/native_window_jni.h>
#include <android/native_window.h>
#include <functional>
#include <thread>

#include "camera_manager.h"

class CameraAppEngine {
public:
    explicit CameraAppEngine(JNIEnv *env, jint w, jint h);

    ~CameraAppEngine();

    // Manage NDKCamera Object
    void CreateCameraSession(jobject surface);

    void StartPreview(bool start);

    const ImageFormat &GetCompatibleCameraRes() const;

    int32_t GetCameraSensorOrientation(int32_t facing);

    jobject GetSurfaceObject();

private:
    JNIEnv *env_;
    int32_t requestWidth_;
    int32_t requestHeight_;
    jobject surface_;
    NDKCamera *camera_;
    ImageFormat compatibleCameraRes_{};
};

#endif

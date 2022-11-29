#include <android/native_window_jni.h> // <jni.h>
#include <cstdio>
#include <cstring>

#include "camera_engine.h"
#include "../global.h"

CameraEngine::CameraEngine(JNIEnv *env, jint w, jint h)
        : env_(env), surface_(nullptr), camera_(nullptr) {
    camera_ = new NDKCamera();
    ASSERT(camera_, "Failed to create CameraObject")

    dimensions_ = std::pair<int32_t, int32_t>(w, h);
    camera_->MatchCaptureSizeRequest(&dimensions_);
    reader_ = new ImageReader(&dimensions_);
}

CameraEngine::~CameraEngine() {
    if (camera_) {
        delete camera_;
        camera_ = nullptr;
    }
    if (reader_) {
        delete reader_;
        reader_ = nullptr;
    }
    if (surface_) {
        env_->DeleteGlobalRef(surface_);
        surface_ = nullptr;
    }
}

void CameraEngine::CreateCameraSession(JNIEnv *env, jobject surface) {
    surface_ = env_->NewGlobalRef(surface);
    ASSERT(reader_, "reader_ is NULL!")
    reader_->SetMirrorWindow(ANativeWindow_fromSurface(env, surface));
    camera_->CreateSession(reader_->GetNativeWindow());
}

/**
 * @param start is true to start preview, false to stop preview
 * @return  true if preview started, false when error happened
 */
void CameraEngine::StartPreview(bool start) { camera_->StartPreview(start); }

bool CameraEngine::SetRecording(bool b) {
    if (reader_)
        return reader_->SetRecording(b);
    else return false;
}

jobject CameraEngine::GetSurfaceObject() { return surface_; }

const std::pair<int32_t, int32_t> &CameraEngine::GetDimensions() const {
    return dimensions_;
}

/*int CameraEngine::GetCameraSensorOrientation(int32_t requestFacing) {
    ASSERT(requestFacing == ACAMERA_LENS_FACING_BACK,
           "Only support rear facing camera")
    int32_t facing = 0, angle = 0;
    if (camera_->GetSensorOrientation(&facing, &angle) ||
        facing == requestFacing) {
        return angle;
    }
    ASSERT(false, "Failed for GetSensorOrientation()")
}*/

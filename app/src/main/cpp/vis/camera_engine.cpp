#include <android/native_window_jni.h>
#include <cstdio>
#include <cstring>

#include "camera_engine.h"
#include "../otr/debug.h"

CameraEngine::CameraEngine(JNIEnv *env, jint w, jint h)
        : env_(env),
          surface_(nullptr),
          camera_(nullptr) {
    camera_ = new NDKCamera();
    ASSERT(camera_, "Failed to Create CameraObject")

    imageFormat = ImageFormat{0, 0, VIS_IMAGE_FORMAT};
    camera_->MatchCaptureSizeRequest(w, h, &imageFormat);
    ASSERT(imageFormat.width && imageFormat.height, "Could not find supportable resolution")

    reader_ = new ImageReader(&imageFormat); // reader_->SetPresentRotation(0);
    reader_->RegisterCallback(
            this, [/*this*/](void *ctx, const char *str) -> void {
                //reinterpret_cast<CameraEngine* >(ctx)->OnPhotoTaken(str);
            });
    camera_->MatchCaptureSizeRequest(w, h, &imageFormat);
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

jobject CameraEngine::GetSurfaceObject() { return surface_; }

const ImageFormat &CameraEngine::GetCompatibleCameraRes() const {
    return imageFormat;
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

/**
 * @param start is true to start preview, false to stop preview
 * @return  true if preview started, false when error happened
 */
void CameraEngine::StartPreview(bool start) { camera_->StartPreview(start); }

#include <cstdio>
#include <cstring>
#include "camera_engine.h"
#include "utils/native_debug.h"

CameraAppEngine::CameraAppEngine(JNIEnv *env, jint w, jint h)
        : env_(env),
          requestWidth_(w),
          requestHeight_(h),
          surface_(nullptr),
          camera_(nullptr) {
    memset(&compatibleCameraRes_, 0, sizeof(compatibleCameraRes_));
    camera_ = new NDKCamera();
    ASSERT(camera_, "Failed to Create CameraObject")
    camera_->MatchCaptureSizeRequest(requestWidth_, requestHeight_,
                                     &compatibleCameraRes_);
}

CameraAppEngine::~CameraAppEngine() {
    if (camera_) {
        delete camera_;
        camera_ = nullptr;
    }

    if (surface_) {
        env_->DeleteGlobalRef(surface_);
        surface_ = nullptr;
    }
}

void CameraAppEngine::CreateCameraSession(jobject surface) {
    surface_ = env_->NewGlobalRef(surface);
    camera_->CreateSession(ANativeWindow_fromSurface(env_, surface));
}

jobject CameraAppEngine::GetSurfaceObject() { return surface_; }

const ImageFormat &CameraAppEngine::GetCompatibleCameraRes() const {
    return compatibleCameraRes_;
}

int CameraAppEngine::GetCameraSensorOrientation(int32_t requestFacing) {
    ASSERT(requestFacing == ACAMERA_LENS_FACING_BACK,
           "Only support rear facing camera")
    int32_t facing = 0, angle = 0;
    if (camera_->GetSensorOrientation(&facing, &angle) ||
        facing == requestFacing) {
        return angle;
    }
    ASSERT(false, "Failed for GetSensorOrientation()")
}

/**
 * @param start is true to start preview, false to stop preview
 * @return  true if preview started, false when error happened
 */
void CameraAppEngine::StartPreview(bool start) { camera_->StartPreview(start); }

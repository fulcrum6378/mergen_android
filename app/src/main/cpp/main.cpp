#include <android_native_app_glue.h>
#include <memory>
#include "camera_manager.cpp"

struct Engine {
    struct android_app *app;
    NDKCamera* camera_;

    void create_camera() {
        // Camera needed to be requested at the run-time from Java SDK
        if (!app->window) return;

        int32_t displayRotation = get_display_rotation();
        rotation_ = displayRotation;

        camera_ = new NDKCamera();

        int32_t facing = 0, angle = 0, imageRotation = 0;
        if (camera_->GetSensorOrientation(&facing, &angle)) {
            if (facing == ACAMERA_LENS_FACING_FRONT) {
                imageRotation = (angle + rotation_) % 360;
                imageRotation = (360 - imageRotation) % 360;
            } else {
                imageRotation = (angle - rotation_ + 360) % 360;
            }
        }
        LOGI("Phone Rotation: %d, Present Rotation Angle: %d", rotation_,
             imageRotation);
        ImageFormat view{0, 0, 0}, capture{0, 0, 0};
        camera_->MatchCaptureSizeRequest(app->window, &view, &capture);

        // Request the necessary nativeWindow to OS
        bool portraitNativeWindow =
                (savedNativeWinRes_.width < savedNativeWinRes_.height);
        ANativeWindow_setBuffersGeometry(
                app->window, portraitNativeWindow ? view.height : view.width,
                portraitNativeWindow ? view.width : view.height, WINDOW_FORMAT_RGBA_8888);

        yuvReader_ = new ImageReader(&view, AIMAGE_FORMAT_YUV_420_888);
        yuvReader_->SetPresentRotation(imageRotation);
        jpgReader_ = new ImageReader(&capture, AIMAGE_FORMAT_JPEG);
        jpgReader_->SetPresentRotation(imageRotation);
        jpgReader_->RegisterCallback(this, [this](void* ctx, const char* str) -> void {
            reinterpret_cast<Engine* >(ctx)->OnPhotoTaken(str);
        });

        // now we could create session
        camera_->CreateSession(yuvReader_->GetNativeWindow(),
                               jpgReader_->GetNativeWindow(), imageRotation);
    }

    void delete_camera() {
        cameraReady_ = false;
        if (camera_) {
            delete camera_;
            camera_ = nullptr;
        }
        if (yuvReader_) {
            delete yuvReader_;
            yuvReader_ = nullptr;
        }
        if (jpgReader_) {
            delete jpgReader_;
            jpgReader_ = nullptr;
        }
    }

    int get_display_rotation() { // TODO FUCKS
        JNIEnv *env;
        ANativeActivity *activity = app_->activity;
        activity->vm->GetEnv((void **)&env, JNI_VERSION_1_6);

        activity->vm->AttachCurrentThread(&env, nullptr);

        jobject activityObj = env->NewGlobalRef(activity->clazz);
        jclass clz = env->GetObjectClass(activityObj);
        jint newOrientation = env->CallIntMethod(
                activityObj, env->GetMethodID(clz, "getRotationDegree", "()I"));
        env->DeleteGlobalRef(activityObj);

        activity->vm->DetachCurrentThread();
        return newOrientation;
    }
};

static int32_t handle_input(struct android_app *app, AInputEvent *event) {
    return 0;
}

static void handle_cmd(struct android_app *app, int32_t cmd) {
}

__attribute__((unused)) void android_main(struct android_app *app) {
    struct Engine myState{};

    memset(&myState, 0, sizeof(myState));
    app->userData = &myState;
    app->onAppCmd = handle_cmd;
    app->onInputEvent = handle_input;
    myState.app = app;
}

extern "C" JNIEXPORT void Java_ir_mahdiparastesh_mergen_Main_preview(
        JNIEnv *env, jobject main) {
}

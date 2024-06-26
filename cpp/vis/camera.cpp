#include <android/asset_manager_jni.h>
#include <thread>

#include "../global.hpp"
#include "camera.hpp"

Camera::Camera(JavaVM *jvm
#if VIS_METHOD == 1
        , jobject main
#elif VIS_METHOD == 2
        , jobject assets
#endif
) : jvm_(jvm), captureSessionState_(CaptureSessionState::MAX_STATE) {
#if VIS_METHOD == 1
    main_ = main;
#endif

    // initialise ACameraManager and get ACameraDevice instances
    cameraMgr_ = ACameraManager_create();
    ASSERT(cameraMgr_, "Failed to create cameraManager")
    cameras_.clear();
    EnumerateCamera();
    ASSERT(!activeCameraId_.empty(), "Unknown ActiveCameraIdx")

    // camera device management
    cameraDeviceListener_ = {
            .context = this,
            .onDisconnected = [](void *ctx, ACameraDevice *dev) {
                auto *cam = reinterpret_cast<Camera *>(ctx);
                std::string id(ACameraDevice_getId(dev));
                LOGW("device %s is disconnected", id.c_str());

                ACameraDevice_close(cam->cameras_[id].device_);
                cam->cameras_.erase(id);
            },
            .onError = nullptr,
    };
    ACameraManager_openCamera(
            cameraMgr_, activeCameraId_.c_str(), &cameraDeviceListener_,
            &cameras_[activeCameraId_].device_);

    // camera state management
    sessionListener = {
            .context = this,
            .onClosed = [](void *ctx, ACameraCaptureSession *ses) {
                reinterpret_cast<Camera *>(ctx)->captureSessionState_ = CaptureSessionState::CLOSED;
            },
            .onReady = [](void *ctx, ACameraCaptureSession *ses) {
                reinterpret_cast<Camera *>(ctx)->captureSessionState_ = CaptureSessionState::READY;
            },
            .onActive = [](void *ctx, ACameraCaptureSession *ses) {
                reinterpret_cast<Camera *>(ctx)->captureSessionState_ = CaptureSessionState::ACTIVE;
            },
    };

    // determine width and height of output images
    DetermineCaptureDimensions();

    // initialise AImageReader and retrieve its ANativeWindow (Surface in Java)
    media_status_t status = AImageReader_newWithUsage(
            dimensions.first, dimensions.second, IMAGE_FORMAT,
            AHARDWAREBUFFER_USAGE_CPU_READ_OFTEN, MAX_BUF_COUNT, &reader_);
    ASSERT(reader_ && status == AMEDIA_OK, "Failed to create AImageReader")
    status = AImageReader_getWindow(reader_, &readerWindow_);
    ASSERT(status == AMEDIA_OK, "Could not get ANativeWindow")

    // listen for incoming image frames
    AImageReader_ImageListener listener{
            .context = this,
            .onImageAvailable = [](void *ctx, AImageReader *reader) {
                reinterpret_cast<Camera *>(ctx)->ImageCallback(reader);
            },
    };
    AImageReader_setImageListener(reader_, &listener);

    // prepare for image analysis
    JNIEnv *env;
    jvm_->GetEnv((void **) &env, JNI_VERSION_1_6);
#if VIS_METHOD == 1
    jmSignal_ = env->GetMethodID(
            env->FindClass("ir/mahdiparastesh/mergen/Main"), "signal", "(B)V");
    segmentation = new Segmentation(jvm, main_, &jmSignal_);
#if VIS_SEG_MARKERS
    segmentation->jmSegMarker = env->GetMethodID(
            env->FindClass("ir/mahdiparastesh/mergen/Main"), "updateSegMarkers", "([J)V");
#endif //VIS_SEG_MARKERS
#elif VIS_METHOD == 2
    segmentation = new Segmentation(AAssetManager_fromJava(env, assets));
#endif //VIS_METHOD
}

/**
 * Loop through cameras on the system, pick up
 * 1) back facing one if available
 * 2) otherwise pick the first one reported to us
 */
void Camera::EnumerateCamera() {
    ACameraIdList *cameraIds = nullptr;
    ACameraManager_getCameraIdList(cameraMgr_, &cameraIds);

    for (int i = 0; i < cameraIds->numCameras; ++i) {
        const char *id = cameraIds->cameraIds[i];

        ACameraMetadata *metadataObj;
        ACameraManager_getCameraCharacteristics(cameraMgr_, id, &metadataObj);

        int32_t count = 0;
        const uint32_t *tags = nullptr;
        ACameraMetadata_getAllTags(metadataObj, &count, &tags);
        for (int tagIdx = 0; tagIdx < count; ++tagIdx)
            if (ACAMERA_LENS_FACING == tags[tagIdx]) {
                ACameraMetadata_const_entry lensInfo = {0};
                ACameraMetadata_getConstEntry(metadataObj, tags[tagIdx], &lensInfo);
                CameraId cam(id);
                cam.facing_ = static_cast<acamera_metadata_enum_android_lens_facing_t>(
                        lensInfo.data.u8[0]);
                cam.device_ = nullptr;
                cameras_[cam.id_] = cam;
                if (cam.facing_ == ACAMERA_LENS_FACING_BACK)
                    activeCameraId_ = cam.id_;
                break;
            }
        ACameraMetadata_free(metadataObj);
    }

    ASSERT(!cameras_.empty(), "No Camera Available on the device")
    if (activeCameraId_.empty()) {
        // if no back facing camera found, pick up the first one to use...
        activeCameraId_ = cameras_.begin()->second.id_;
    }
    ACameraManager_deleteCameraIdList(cameraIds);
}

void Camera::DetermineCaptureDimensions() {
    ACameraMetadata *metadata;
    ACameraManager_getCameraCharacteristics(
            cameraMgr_, activeCameraId_.c_str(), &metadata);
    ACameraMetadata_const_entry entry;
    ACameraMetadata_getConstEntry(
            metadata, ACAMERA_SCALER_AVAILABLE_STREAM_CONFIGURATIONS, &entry);
    // format of the data: format, width, height, input?, type int32
    bool foundIt = false;
    dimensions = std::pair<int16_t, int16_t>(720, 720);
    auto nearestHeight = static_cast<int32_t>(H);

    for (uint32_t i = 0u; i < entry.count; i += 4u) {
        int32_t input = entry.data.i32[i + 3];
        int32_t format = entry.data.i32[i + 0];
        if (input || format != IMAGE_FORMAT) continue;

        std::pair<int32_t, int32_t> ent(entry.data.i32[i + 1], entry.data.i32[i + 2]);
        //if (ent.first == ent.second) continue; // discard square dimensions

        if (abs(ent.second - nearestHeight) < abs(dimensions.second - nearestHeight)) {
            foundIt = true;
            dimensions = ent;
        }
    }
    if (!foundIt) LOGW("Did not find any compatible camera resolution, taking 720x720");
}

/*int NDKCamera::GetCameraSensorOrientation(int8_t requestFacing) {
    if (!cameraMgr_) return false;
    ASSERT(requestFacing == ACAMERA_LENS_FACING_BACK, "Only support rear facing camera")
    int32_t facing, angle;

    ACameraMetadata *metadataObj;
    ACameraMetadata_const_entry face, orientation;
    ACameraManager_getCameraCharacteristics(
            cameraMgr_, activeCameraId_.c_str(), &metadataObj);
    ACameraMetadata_getConstEntry(metadataObj, ACAMERA_LENS_FACING, &face);
    facing = static_cast<int32_t>(face.data.u8[0]);

    ACameraMetadata_getConstEntry(
            metadataObj, ACAMERA_SENSOR_ORIENTATION, &orientation);
    ACameraMetadata_free(metadataObj);
    angle = orientation.data.i32[0];

    if (facing == requestFacing) return angle;
    ASSERT(false, "Failed for GetSensorOrientation()")
}*/

/** My phone gives 8..10 FPS (1088x1088)!
 * In order to implement ANativeWindow_setFrameRate(), you'll need to add "nativewindow" to
 * target_link_libraries() in CMakeLists. But it didn't make any difference in my phone! */
void Camera::CreateSession(ANativeWindow *displayWindow) {
    displayWindow_ = displayWindow;
    ACaptureSessionOutputContainer_create(&outputContainer_);

    // READER
    ANativeWindow_acquire(readerWindow_);
    ACaptureSessionOutput_create(readerWindow_, &readerOutput_);
    ACaptureSessionOutputContainer_add(outputContainer_, readerOutput_);
    ACameraOutputTarget_create(readerWindow_, &readerTarget_);
    ACameraDevice_createCaptureRequest(cameras_[activeCameraId_].device_,
                                       TEMPLATE_PREVIEW, &readerRequest_);
    ACaptureRequest_addTarget(readerRequest_, readerTarget_);

    // DISPLAY
    ANativeWindow_acquire(displayWindow_);
    ACaptureSessionOutput_create(displayWindow_, &displayOutput_);
    ACaptureSessionOutputContainer_add(outputContainer_, displayOutput_);
    ACameraOutputTarget_create(displayWindow_, &displayTarget_);
    ACameraDevice_createCaptureRequest(cameras_[activeCameraId_].device_,
                                       TEMPLATE_PREVIEW, &displayRequest_);
    ACaptureRequest_addTarget(displayRequest_, displayTarget_);

    captureSessionState_ = CaptureSessionState::READY;
    ACameraDevice_createCaptureSession(
            cameras_[activeCameraId_].device_, outputContainer_,
            &sessionListener, &captureSession_);
    Preview(true);
}

void Camera::Preview(bool start) {
    if (start)
        ACameraCaptureSession_setRepeatingRequest(
                captureSession_, nullptr, 2,
                new ACaptureRequest *[2]{readerRequest_, displayRequest_},
                nullptr);
    else if (captureSessionState_ == CaptureSessionState::ACTIVE)
        ACameraCaptureSession_stopRepeating(captureSession_);
}

/** Always use the camera while rotating the phone in -90 degrees. */
bool Camera::SetRecording(bool b) {
    if (captureSessionState_ != CaptureSessionState::ACTIVE || b == recording_) return false;
    recording_ = b;
#if VIS_METHOD == 0
    if (b) bmp_stream_ = new BitmapStream(dimensions); else delete bmp_stream_;
    // if (bmp_stream_) bmp_stream_->BakeMetadata();
#endif
    return true;
}

/**
 * Called when a frame is captured.
 * Beware that AImageReader_acquireLatestImage deletes the previous images.
 * You should always call acquireNextImage() and delete() even if you don't wanna save it!
 * Make sure you create a separate thread for any kind of heavy work, or they'll make the preview lag!
 *
 * AImage_getTimestamp sucks! e.g. gives "1968167967185224" for 2023.06.28!
 */
void Camera::ImageCallback(AImageReader *reader) {
    AImage *image = nullptr;
    if (AImageReader_acquireNextImage(reader, &image) != AMEDIA_OK || !image) return;
    bool used = false;
    if (recording_) {
#if VIS_METHOD == 0
        used = bmp_stream_->HandleImage(image);
#else
        used = !segmentation->locked;
        if (used)
            std::thread(&Segmentation::Process, segmentation, image
#if VIS_METHOD == 1
                    , &recording_
#endif
            ).detach();
#endif
    }
    if (!used) AImage_delete(image);
}

Camera::~Camera() {
    Preview(false);
    ACameraCaptureSession_close(captureSession_);

    // destroy objects related to image analysis
#if VIS_METHOD >= 1
    delete segmentation;
    segmentation = nullptr;
#endif

    // destroy camera session
    if (readerWindow_) {
        ACaptureRequest_removeTarget(readerRequest_, readerTarget_);
        ACaptureRequest_free(readerRequest_);
        ACameraOutputTarget_free(readerTarget_);

        ACaptureSessionOutputContainer_remove(outputContainer_, readerOutput_);
        ACaptureSessionOutput_free(readerOutput_);

        ANativeWindow_release(readerWindow_);
    }
    if (displayWindow_) {
        ACaptureRequest_removeTarget(displayRequest_, displayTarget_);
        ACaptureRequest_free(displayRequest_);
        ACameraOutputTarget_free(displayTarget_);

        ACaptureSessionOutputContainer_remove(outputContainer_, displayOutput_);
        ACaptureSessionOutput_free(displayOutput_);

        ANativeWindow_release(displayWindow_);
    }
    ACaptureSessionOutputContainer_free(outputContainer_);

    // destroy AImageReader
    AImageReader_delete(reader_);
    reader_ = nullptr;

    // destroy ACameraDevice instances and ACameraManager
    for (auto &cam: cameras_)
        if (cam.second.device_) ACameraDevice_close(cam.second.device_);
    cameras_.clear();
    ACameraManager_delete(cameraMgr_);
    cameraMgr_ = nullptr;
}

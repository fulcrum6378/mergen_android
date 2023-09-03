#include <cstdlib>
#include <dirent.h>
#include <fstream>
#include <thread>

#include "camera.h"
#include "../global.h"

/**
 * This should be invoked before the first Main::onSurfaceTextureAvailable;
 * its return value will be needed for Main::getCameraDimensions, then we should have some
 * configurations done and then we'll create a camera session.
 */
Camera::Camera() : store(nullptr), captureSessionState_(CaptureSessionState::MAX_STATE) {

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
    DetermineCaptureDimensions(&dimensions_);

    // initialise AImageReader and retrieve its ANativeWindow (Surface in Java)
    media_status_t status = AImageReader_newWithUsage(
            dimensions_.first, dimensions_.second, VIS_IMAGE_FORMAT,
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
}

const std::pair<int32_t, int32_t> &Camera::GetDimensions() const {
    return dimensions_;
}

[[maybe_unused]] void Camera::BakeMetadata() const {
    int32_t width = dimensions_.first;
    int32_t height = dimensions_.second;
    std::ofstream metadata(
            filesDir + std::to_string(width) + "x" + std::to_string(height),
            std::ios::out | std::ios::binary);

    bmpfile_magic magic{'B', 'M'};
    metadata.write((char *) (&magic), sizeof(magic));
    bmpfile_header header = {0};
    header.bmp_offset = sizeof(bmpfile_magic) + sizeof(bmpfile_header) + sizeof(bmpfile_dib_info);
    header.file_size = header.bmp_offset + (height * 3 + width % 4) * height;
    metadata.write((char *) (&header), sizeof(header));
    bmpfile_dib_info dib_info = {0};
    dib_info.header_size = sizeof(bmpfile_dib_info);
    dib_info.width = width;
    dib_info.height = height;
    dib_info.num_planes = 1;
    dib_info.bits_per_pixel = 24;
    dib_info.compression = 0;
    dib_info.bmp_byte_size = 0;
    dib_info.hres = 2835;
    dib_info.vres = 2835;
    dib_info.num_colors = 0;
    dib_info.num_important_colors = 0;
    metadata.write((char *) (&dib_info), sizeof(dib_info));
    metadata.close();
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
    if (activeCameraId_.length() == 0) {
        // if no back facing camera found, pick up the first one to use...
        activeCameraId_ = cameras_.begin()->second.id_;
    }
    ACameraManager_deleteCameraIdList(cameraIds);
}

bool Camera::DetermineCaptureDimensions(std::pair<int32_t, int32_t> *dimen) {
    ACameraMetadata *metadata;
    ACameraManager_getCameraCharacteristics(
            cameraMgr_, activeCameraId_.c_str(), &metadata);
    ACameraMetadata_const_entry entry;
    ACameraMetadata_getConstEntry(
            metadata, ACAMERA_SCALER_AVAILABLE_STREAM_CONFIGURATIONS, &entry);
    // format of the data: format, width, height, input?, type int32
    bool foundIt = false;
    std::pair<int32_t, int32_t> ultimate(720, 720);

    for (uint32_t i = 0; i < entry.count; i += 4) {
        int32_t input = entry.data.i32[i + 3];
        int32_t format = entry.data.i32[i + 0];
        if (input || format != VIS_IMAGE_FORMAT) continue;

        std::pair<int32_t, int32_t> ent(entry.data.i32[i + 1], entry.data.i32[i + 2]);
        //if (ent.first == ent.second) continue; // discard square dimensions

        if (abs(ent.second - VIS_IMAGE_NEAREST_HEIGHT) <
            abs(ultimate.second - VIS_IMAGE_NEAREST_HEIGHT)) {
            foundIt = true;
            ultimate = ent;
        }
    }
    if (!foundIt) LOGW("Did not find any compatible camera resolution, taking 720x720");

    *dimen = ultimate;
    return foundIt;
}

/*int NDKCamera::GetCameraSensorOrientation(int32_t requestFacing) {
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

/** The default FPS is in range 22..24! */
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

bool Camera::SetRecording(bool b) {
    if (captureSessionState_ != CaptureSessionState::ACTIVE || b == recording_) return false;
    if (b)
        store = new std::ofstream(
                (filesDir + std::string("vis.rgb")).c_str(),
                std::ios::out | std::ios::binary);
    recording_ = b;
    if (!recording_) skipped_count = 0;
    return true;
}

/**
 * Called when a frame is captured
 * Beware that AImageReader_acquireLatestImage deletes the previous images.
 * You should always call acquireNextImage() and delete() even if you don't wanna save it!
 *
 * AImage_getTimestamp sucks! e.g. gives "1968167967185224" for 2023.06.28!
 */
void Camera::ImageCallback(AImageReader *reader) {
    AImage *image = nullptr;
    if (AImageReader_acquireNextImage(reader, &image) != AMEDIA_OK || !image) return;
    bool submitted = false;
    if (recording_) {
        if (skipped_count == 0) {
            std::thread(&Camera::Submit, this, image).detach();
            submitted = true;
        }
        skipped_count++;
        if (skipped_count == VIS_SKIP_N_FRAMES) skipped_count = 0;
    }
    if (!submitted) AImage_delete(image);
}

/**
 * Yuv2Rgb algorithm is from:
 * https://github.com/tensorflow/tensorflow/blob/5dcfc51118817f27fad5246812d83e5dccdc5f72/
 * tensorflow/tools/android/test/jni/yuv2rgb.cc
 *
 * TODO Images must be rotated by +90 degrees
 */
void Camera::Submit(AImage *image) {
    store_mutex.lock();
    if (!recording_ || !store) {
        if (store) {
            store->close();
            delete &store;
            store = nullptr;
        }
        store_mutex.unlock();
        AImage_delete(image);
        return;
    }

    AImageCropRect srcRect;
    AImage_getCropRect(image, &srcRect);
    int32_t yStride, uvStride;
    uint8_t *yPixel, *uPixel, *vPixel;
    int32_t yLen, uLen, vLen;
    AImage_getPlaneRowStride(image, 0, &yStride);
    AImage_getPlaneRowStride(image, 1, &uvStride);
    AImage_getPlaneData(image, 0, &yPixel, &yLen);
    AImage_getPlaneData(image, 1, &vPixel, &vLen);
    AImage_getPlaneData(image, 2, &uPixel, &uLen);
    int32_t uvPixelStride;
    AImage_getPlanePixelStride(image, 1, &uvPixelStride);
    int32_t width = dimensions_.first, height = dimensions_.second;
    /*AImage_getWidth(image, &width);
    AImage_getHeight(image, &height);
    height = MIN(width, (srcRect.bottom - srcRect.top));
    width = MIN(height, (srcRect.right - srcRect.left));*/

    for (int32_t y = height - 1; y >= 0; y--) {
        const uint8_t *pY = yPixel + yStride * (y + srcRect.top) + srcRect.left;

        int32_t uv_row_start = uvStride * ((y + srcRect.top) >> 1);
        const uint8_t *pU = uPixel + uv_row_start + (srcRect.left >> 1);
        const uint8_t *pV = vPixel + uv_row_start + (srcRect.left >> 1);

        for (int32_t x = 0; x < width; x++) {
            const int32_t uv_offset = (x >> 1) * uvPixelStride;

            int nY = pY[x] - 16;
            int nU = pU[uv_offset] - 128;
            int nV = pV[uv_offset] - 128;
            if (nY < 0) nY = 0;

            int nR = (int) (1192 * nY + 1634 * nV);
            int nG = (int) (1192 * nY - 833 * nV - 400 * nU);
            int nB = (int) (1192 * nY + 2066 * nU);

            int maxR = MAX(0, nR), maxG = MAX(0, nG), maxB = MAX(0, nB);
            nR = MIN(kMaxChannelValue, maxR);
            nG = MIN(kMaxChannelValue, maxG);
            nB = MIN(kMaxChannelValue, maxB);

            nR = (nR >> 10) & 0xff;
            nG = (nG >> 10) & 0xff;
            nB = (nB >> 10) & 0xff;

            store->put((char) nR);
            store->put((char) nG);
            store->put((char) nB);
        }
        for (int i = 0; i < width % 4; i++) store->put(0);
    }
    /*LOGE("%s", (std::to_string(store->tellp()) +
                " bytes - dimensions: " + std::to_string(width) + "x" + std::to_string(height)
    ).c_str());*/
    store_mutex.unlock();

    //queuer_->Input(INPUT_ID_BACK_LENS, data, time);
    AImage_delete(image);
}

Camera::~Camera() {
    Preview(false);
    ACameraCaptureSession_close(captureSession_);

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

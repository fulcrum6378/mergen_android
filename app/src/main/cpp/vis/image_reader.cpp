#include <cstdlib>
#include <ctime>
#include <dirent.h>
#include <functional>
#include <media/NdkImageReader.h>
#include <string>
#include <thread>
#include <utility>

#include "image_reader.h"
#include "../otr/debug.h"

static const char *path = "/data/data/ir.mahdiparastesh.mergen/files/";

#define MAX_BUF_COUNT 4 // max buffers in this ImageReader

/**
 * ImageReader listener: called by AImageReader for every frame captured
 * We pass the event to ImageReader class, so it could do some housekeeping
 * about the loaded queue. For example, we could keep a counter to track how
 * many buffers are full and idle in the queue. If camera almost has no buffer
 * to capture we could release ( skip ) some frames by AImageReader_getNextImage()
 * and AImageReader_delete().
 */
void OnImageCallback(void *ctx, AImageReader *reader) {
    reinterpret_cast<ImageReader *>(ctx)->ImageCallback(reader);
}

ImageReader::ImageReader(ImageFormat *format) : reader_(nullptr), mirror_(nullptr) {
    callback_ = nullptr;
    callbackCtx_ = nullptr;

    media_status_t status = AImageReader_new(
            format->width, format->height, format->format,
            MAX_BUF_COUNT, &reader_);
    ASSERT(reader_ && status == AMEDIA_OK, "Failed to create AImageReader")

    AImageReader_ImageListener listener{
            .context = this,
            .onImageAvailable = OnImageCallback,
    };
    AImageReader_setImageListener(reader_, &listener);
}

ImageReader::~ImageReader() {
    ASSERT(reader_, "NULL Pointer to %s", __FUNCTION__)
    AImageReader_delete(reader_);
}

void ImageReader::RegisterCallback(
        void *ctx, std::function<void(void *ctx, const char *fileName)> func) {
    callbackCtx_ = ctx;
    callback_ = std::move(func);
}

void ImageReader::ImageCallback(AImageReader *reader) {
    AImage *image = nullptr;
    media_status_t status = AImageReader_acquireNextImage(reader, &image);
    ASSERT(status == AMEDIA_OK && image, "Image is not available")

    std::thread writeFileHandler(&ImageReader::WriteFile, this, image);
    writeFileHandler.detach();
}

void ImageReader::SetMirrorWindow(ANativeWindow *window) {
    mirror_ = window;
}

ANativeWindow *ImageReader::GetNativeWindow() {
    if (!reader_) return nullptr;
    ANativeWindow *nativeWindow;
    media_status_t status = AImageReader_getWindow(reader_, &nativeWindow);
    ASSERT(status == AMEDIA_OK, "Could not get ANativeWindow")
    return nativeWindow;
}

AImage *ImageReader::GetNextImage() {
    AImage *image;
    media_status_t status = AImageReader_acquireNextImage(reader_, &image);
    if (status != AMEDIA_OK) return nullptr;
    return image;
}

AImage *ImageReader::GetLatestImage() {
    AImage *image;
    media_status_t status = AImageReader_acquireLatestImage(reader_, &image);
    if (status != AMEDIA_OK) return nullptr;
    return image;
}

void ImageReader::DeleteImage(AImage *image) {
    if (image) AImage_delete(image);
}

static const int kMaxChannelValue = 262143;

/**
 * Helper function for YUV_420 to RGB conversion. Courtesy of Tensorflow
 * ImageClassifier Sample:
 * https://github.com/tensorflow/tensorflow/blob/master/tensorflow/examples/android/jni/yuv2rgb.cc
 * The difference is that here we have to swap UV plane when calling it.
 *
 * This value is 2 ^ 18 - 1, and is used to clamp the RGB values before their ranges are normalized
 * to eight bits.
 */
static inline uint32_t YUV2RGB(int nY, int nU, int nV) {
    nY -= 16;
    nU -= 128;
    nV -= 128;
    if (nY < 0) nY = 0;

    // This is the floating point equivalent. We do the conversion in integer
    // because some Android devices do not have floating point in hardware.
    // nR = (int)(1.164 * nY + 1.596 * nV);
    // nG = (int)(1.164 * nY - 0.813 * nV - 0.391 * nU);
    // nB = (int)(1.164 * nY + 2.018 * nU);

    int nR = (int) (1192 * nY + 1634 * nV);
    int nG = (int) (1192 * nY - 833 * nV - 400 * nU);
    int nB = (int) (1192 * nY + 2066 * nU);

    nR = MIN(kMaxChannelValue, MAX(0, nR));
    nG = MIN(kMaxChannelValue, MAX(0, nG));
    nB = MIN(kMaxChannelValue, MAX(0, nB));

    nR = (nR >> 10) & 0xff;
    nG = (nG >> 10) & 0xff;
    nB = (nB >> 10) & 0xff;

    return 0xff000000 | (nR << 16) | (nG << 8) | nB;
}

bool ImageReader::DisplayImage(ANativeWindow_Buffer *buf, AImage *image) {
    ASSERT(buf->format == WINDOW_FORMAT_RGBX_8888 ||
           buf->format == WINDOW_FORMAT_RGBA_8888,
           "Not supported buffer format")

    int32_t srcFormat = -1;
    AImage_getFormat(image, &srcFormat);
    ASSERT(AIMAGE_FORMAT_YUV_420_888 == srcFormat, "Failed to get format")
    int32_t srcPlanes = 0;
    AImage_getNumberOfPlanes(image, &srcPlanes);
    ASSERT(srcPlanes == 3, "Is not 3 planes")

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

    int32_t height = MIN(buf->height, (srcRect.bottom - srcRect.top));
    int32_t width = MIN(buf->width, (srcRect.right - srcRect.left));

    auto *out = static_cast<uint32_t *>(buf->bits);
    for (int32_t y = 0; y < height; y++) {
        const uint8_t *pY = yPixel + yStride * (y + srcRect.top) + srcRect.left;

        int32_t uv_row_start = uvStride * ((y + srcRect.top) >> 1);
        const uint8_t *pU = uPixel + uv_row_start + (srcRect.left >> 1);
        const uint8_t *pV = vPixel + uv_row_start + (srcRect.left >> 1);

        for (int32_t x = 0; x < width; x++) {
            const int32_t uv_offset = (x >> 1) * uvPixelStride;
            out[x] = YUV2RGB(pY[x], pU[uv_offset], pV[uv_offset]);
        }
        out += buf->stride;
    }

    AImage_delete(image);

    return true;
}

void ImageReader::WriteFile(AImage *image) {
    /*int planeCount;
    media_status_t status = AImage_getNumberOfPlanes(image, &planeCount);
    FIXME ASSERT(status == AMEDIA_OK && planeCount == 1,
      "Error: getNumberOfPlanes() planeCount = %d", planeCount);*/

    uint8_t *data = nullptr;
    int len = 0;
    AImage_getPlaneData(image, 0, &data, &len);

    DIR *dir = opendir(path);
    if (dir) closedir(dir);
    else {
        std::string cmd = "mkdir -p ";
        cmd += path;
        system(cmd.c_str());
    }

    struct timespec ts{0, 0};
    clock_gettime(CLOCK_REALTIME, &ts);
    struct tm localTime{};
    localtime_r(&ts.tv_sec, &localTime);

    std::string fileName = path;
    std::string dash("-");
    fileName += "capture" + std::to_string(localTime.tm_mon) +
                std::to_string(localTime.tm_mday) + dash +
                std::to_string(localTime.tm_hour) +
                std::to_string(localTime.tm_min) +
                std::to_string(localTime.tm_sec) + ".jpg"; // yuv
    // FIXME THIS PATTERN OF FILENAME MADE THEM BE RE-WRITTEN!
    FILE *file = fopen(fileName.c_str(), "wb");
    if (file && data && len) {
        fwrite(data, 1, len, file);
        fclose(file);

        if (callback_) callback_(callbackCtx_, fileName.c_str());
    } else {
        if (file) fclose(file);
    }
    DeleteImage(image);
}

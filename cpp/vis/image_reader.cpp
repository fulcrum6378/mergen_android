#include <dirent.h>
#include <thread>

#include "cutils/atomic.h"
#include "cutils/log.h"

#include "image_reader.h"
#include "../global.h"

static const char *path = "/data/data/ir.mahdiparastesh.mergen/files/vis/";

static int32_t createProcessUniqueId() { // FIXME
    static volatile int32_t globalCounter = 0;
    return android_atomic_inc(&globalCounter);
}
const char* ImageReader::kCallbackFpKey = "Callback";
const char* ImageReader::kContextKey    = "Context";
const char* ImageReader::kGraphicBufferKey = "GraphicBuffer";

ImageReader::ImageReader(std::pair<int32_t, int32_t> *dimen, Queuer *) :
        reader_(nullptr), queuer_() {
    media_status_t status = AImageReader_newWithUsage(
            dimen->first, dimen->second, VIS_IMAGE_FORMAT,
            AHARDWAREBUFFER_USAGE_CPU_READ_OFTEN, MAX_BUF_COUNT, &reader_);
    ASSERT(reader_ && status == AMEDIA_OK, "Failed to create AImageReader")

    // Create the destination directory
    DIR *dir = opendir(path);
    if (dir) closedir(dir);
    else {
        std::string cmd = "mkdir -p ";
        cmd += path;
        system(cmd.c_str());
    }

    ImageReader_ImageListener listener{
            .context = this,
            .onImageAvailable = [](void *ctx, ImageReader *reader) {
                //reinterpret_cast<ImageReader *>(ctx)->ImageCallback(reader);
            },
    };
    //AImageReader_setImageListener(reader_, &listener);


    //// FIXME
    const int32_t mWidth = dimen->first;
    const int32_t mHeight = dimen->second;
    const int32_t mFormat = VIS_IMAGE_FORMAT;
    const uint64_t mUsage = AHARDWAREBUFFER_USAGE_CPU_READ_OFTEN;  // AHARDWAREBUFFER_USAGE_* flags.
    const int32_t mMaxImages = MAX_BUF_COUNT;

    mFrameListener = new FrameListener(this);
    mFrameListener->setImageListener(&listener);

    auto publicFormat = static_cast<PublicFormat>(mFormat);
    int mHalFormat = mapPublicFormatToHalFormat(publicFormat);
    android_dataspace_t mHalDataSpace = mapPublicFormatToHalDataspace(publicFormat);
    uint64_t mHalUsage = AHardwareBuffer_convertToGrallocUsageBits(
            AHARDWAREBUFFER_USAGE_CPU_READ_OFTEN);
    sp<IGraphicBufferProducer> gbProducer;
    sp<IGraphicBufferConsumer> gbConsumer;
    BufferQueue::createBufferQueue(&gbProducer, &gbConsumer);
    String8 consumerName = String8::format("ImageReader-%dx%df%xu%" PRIu64 "m%d-%d-%d",
                                           mWidth, mHeight, mFormat, mUsage, mMaxImages, getpid(),
                                           createProcessUniqueId());
    mBufferItemConsumer = new BufferItemConsumer(
            gbConsumer, mHalUsage, MAX_BUF_COUNT,
            /*controlledByApp*/ true);
    if (mBufferItemConsumer == nullptr) {
        ALOGE("Failed to allocate BufferItemConsumer");
        //return AMEDIA_ERROR_UNKNOWN;
    }
    sp<IGraphicBufferProducer> mProducer = gbProducer;
    mBufferItemConsumer->setName(consumerName);
    mBufferItemConsumer->setFrameAvailableListener(mFrameListener);
    mBufferItemConsumer->setBufferFreedListener(mBufferRemovedListener);
    status_t res;
    res = mBufferItemConsumer->setDefaultBufferSize(mWidth, mHeight);
    if (res != OK) {
        ALOGE("Failed to set BufferItemConsumer buffer size");
        //return AMEDIA_ERROR_UNKNOWN;
    }
    res = mBufferItemConsumer->setDefaultBufferFormat(mHalFormat);
    if (res != OK) {
        ALOGE("Failed to set BufferItemConsumer buffer format");
        //return AMEDIA_ERROR_UNKNOWN;
    }
    res = mBufferItemConsumer->setDefaultBufferDataSpace(mHalDataSpace);
    if (res != OK) {
        ALOGE("Failed to set BufferItemConsumer buffer dataSpace");
        //return AMEDIA_ERROR_UNKNOWN;
    }
    if (mUsage & AHARDWAREBUFFER_USAGE_PROTECTED_CONTENT) {
        gbConsumer->setConsumerIsProtected(true);
    }
    sp<ANativeWindow> mWindow = TODO();

    mCbLooper = new ALooper();
    //mCbLooper->setName(consumerName.string());
    res = mCbLooper->start(
            /*runOnCallingThread*/false,
            /*canCallJava*/       true,
                                  PRIORITY_DEFAULT);
    if (res != OK) {
        ALOGE("Failed to start the looper");
        //return AMEDIA_ERROR_UNKNOWN;
    }
    mHandler = new CallbackHandler(this);
    mCbLooper->registerHandler(mHandler);

    // FIXME mBufferItemConsumer has 3 more usages in acquireImageLocked and releaseImageLocked
}

// called when a frame is captured
void ImageReader::ImageCallback(AImageReader *reader) {
    AImage *image = nullptr;
    if (AImageReader_acquireNextImage(reader, &image) != AMEDIA_OK || !image) return;

    /*ANativeWindow_acquire(mirror_);
    ANativeWindow_Buffer buf;
    if (ANativeWindow_lock(mirror_, &buf, nullptr) == 0) {
        Mirror(&buf, image);
        ANativeWindow_unlockAndPost(mirror_);
        ANativeWindow_release(mirror_);
    }*/

    if (!recording_) AImage_delete(image);
    else std::thread(&ImageReader::Submit, this, image, Queuer::Now()).detach();
}

ANativeWindow *ImageReader::GetNativeWindow() {
    if (!reader_) return nullptr;
    ANativeWindow *nativeWindow;
    media_status_t status = AImageReader_getWindow(reader_, &nativeWindow);
    ASSERT(status == AMEDIA_OK, "Could not get ANativeWindow")
    return nativeWindow;
}

static const int kMaxChannelValue = 262143;

/**
 * Helper function for YUV_420 to RGB conversion. Courtesy of Tensorflow ImageClassifier Sample:
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

    int maxR = MAX(0, nR), maxG = MAX(0, nG), maxB = MAX(0, nB);
    // inlining these will cause mere-IDE error
    nR = MIN(kMaxChannelValue, maxR);
    nG = MIN(kMaxChannelValue, maxG);
    nB = MIN(kMaxChannelValue, maxB);

    nR = (nR >> 10) & 0xff;
    nG = (nG >> 10) & 0xff;
    nB = (nB >> 10) & 0xff;

    return 0xff000000 | (nR << 16) | (nG << 8) | nB;
}

/**
 * Present camera image to the given display buffer. Avaliable image is converted
 *   to display buffer format. Supported display format:
 *      WINDOW_FORMAT_RGBX_8888
 *      WINDOW_FORMAT_RGBA_8888
 *   @param buf {@link ANativeWindow_Buffer} for image to display to.
 *   @param image a {@link AImage} instance, source of image conversion.
 *            it will be deleted via {@link AImage_delete}
 */
void ImageReader::Mirror(ANativeWindow_Buffer *buf, AImage *image) {
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

    int32_t height = MIN(buf->width, (srcRect.bottom - srcRect.top));
    int32_t width = MIN(buf->height, (srcRect.right - srcRect.left));

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
}

bool ImageReader::SetRecording(bool b) {
    if (b == recording_) return false;
    recording_ = b;
    return true;
}

void ImageReader::Submit(AImage *image, int64_t time) {
    uint8_t *data = nullptr;
    int len = 0;
    AImage_getPlaneData(image, 0, &data, &len);

    std::string fileName = path + std::to_string(time) + ".yuv";
    FILE *file = fopen(fileName.c_str(), "wb");
    if (file && data && len) {
        fwrite(data, 1, len, file);
        //queuer_->Input(SENSE_ID_BACK_LENS, data, time);
        fclose(file);
    } else {
        if (file) fclose(file);
    }
    AImage_delete(image);
}

ImageReader::~ImageReader() {
    if (mBufferItemConsumer != nullptr) {
        mBufferItemConsumer->abandon();
        mBufferItemConsumer->setFrameAvailableListener(nullptr);
    }

    //// FIXME

    ASSERT(reader_, "NULL Pointer to %s", __FUNCTION__)
    AImageReader_delete(reader_);
}


void
ImageReader::FrameListener::onFrameAvailable(const BufferItem& /*item*/) {
    sp<ImageReader> reader = mReader.promote();
    if (reader == nullptr) {
        ALOGW("A frame is available after ImageReader closed!");
        return; // reader has been closed
    }
    Mutex::Autolock _l(mLock);
    if (mListener.onImageAvailable == nullptr) {
        return; // No callback registered
    }
    sp<AMessage> msg = new AMessage(ImageReader::kWhatImageAvailable, reader->mHandler);
    msg->setPointer(ImageReader::kCallbackFpKey, (void *) mListener.onImageAvailable);
    msg->setPointer(ImageReader::kContextKey, mListener.context);
    msg->post();
}
media_status_t
ImageReader::FrameListener::setImageListener(ImageReader_ImageListener* listener) {
    Mutex::Autolock _l(mLock);
    if (listener == nullptr) {
        mListener.context = nullptr;
        mListener.onImageAvailable = nullptr;
    } else {
        mListener = *listener;
    }
    return AMEDIA_OK;
}

void
ImageReader::BufferRemovedListener::onBufferFreed(const wp<GraphicBuffer>& graphicBuffer) {
    sp<ImageReader> reader = mReader.promote();
    if (reader == nullptr) {
        ALOGW("A frame is available after ImageReader closed!");
        return; // reader has been closed
    }
    Mutex::Autolock _l(mLock);
    if (mListener.onBufferRemoved == nullptr) {
        return; // No callback registered
    }
    sp<GraphicBuffer> gBuffer = graphicBuffer.promote();
    if (gBuffer == nullptr) {
        ALOGW("A buffer being freed has gone away!");
        return; // buffer is already destroyed
    }
    sp<AMessage> msg = new AMessage(ImageReader::kWhatBufferRemoved, reader->mHandler);
    msg->setPointer(
            ImageReader::kCallbackFpKey, (void*) mListener.onBufferRemoved);
    msg->setPointer(ImageReader::kContextKey, mListener.context);
    msg->setObject(ImageReader::kGraphicBufferKey, gBuffer);
    msg->post();
}
media_status_t
ImageReader::BufferRemovedListener::setBufferRemovedListener(
        ImageReader_BufferRemovedListener* listener) {
    Mutex::Autolock _l(mLock);
    if (listener == nullptr) {
        mListener.context = nullptr;
        mListener.onBufferRemoved = nullptr;
    } else {
        mListener = *listener;
    }
    return AMEDIA_OK;
}

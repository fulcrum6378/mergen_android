#ifndef VIS_IMAGE_READER_H
#define VIS_IMAGE_READER_H

#include <media/NdkImage.h>
#include <media/NdkImageReader.h>

#include "utils/List.h"
#include "utils/Mutex.h"
#include "utils/StrongPointer.h"

#include "gui/BufferItemConsumer.h"

#include <android/data_space.h>
#include "media/stagefright/foundation/ALooper.h"
#include "media/stagefright/foundation/AMessage.h"
#include "media/stagefright/foundation/AHandler.h"

#include "AHardwareBufferHelpers.h"
#include "ui/PublicFormat.h"

#include <utility>

#include "../mem/queuer.h"

// together with AIMAGE_FORMAT_JPEG, these are the only supported options for my phone apparently!
#define VIS_IMAGE_FORMAT AIMAGE_FORMAT_YUV_420_888
#define SENSE_ID_BACK_LENS 1
// TextureView gets fucked up when height of buffer is larger than its height.
#define VIS_IMAGE_NEAREST_HEIGHT 800 // it would result in 1024x768 in Galaxy A50.

#define MAX_BUF_COUNT 4 // max image buffers
#define MIN(a, b)           \
  ({                        \
    __typeof__(a) _a = (a); \
    __typeof__(b) _b = (b); \
    _a < _b ? _a : _b;      \
  })
#define MAX(a, b)           \
  ({                        \
    __typeof__(a) _a = (a); \
    __typeof__(b) _b = (b); \
    _a > _b ? _a : _b;      \
  })

using namespace android;
class ImageReader;

typedef void (*ImageReader_BufferRemovedCallback)(
        void* context,ImageReader* reader,AHardwareBuffer* buffer);
typedef struct ImageReader_BufferRemovedListener {
    void*                      context;
    ImageReader_BufferRemovedCallback onBufferRemoved;
} ImageReader_BufferRemovedListener;

typedef void (*ImageReader_ImageCallback)(void* context, ImageReader* reader);
typedef struct ImageReader_ImageListener {
    void*                      context;
    ImageReader_ImageCallback onImageAvailable;
} ImageReader_ImageListener;

class ImageReader : public RefBase {
public:
    explicit ImageReader(std::pair<int32_t, int32_t> *dimen, Queuer *queuer);

    ANativeWindow *GetNativeWindow(void);

    void ImageCallback(AImageReader *reader);

    static void Mirror(ANativeWindow_Buffer *buf, AImage *image);

    bool SetRecording(bool b);

    void Submit(AImage *image, int64_t time);

    ~ImageReader();

private:
    /**
     * AImageReader creates an IGraphicBufferProducer (input) and an IGraphicBufferConsumer (output),
     * then it creates a BufferQueue from them, then it listens data from that IGraphicBufferConsumer,
     * THEN IT CREATES A Surface OUT OF THE IGraphicBufferProducer (input), which is the same as
     * ANativeWindow; so AImageReader has no choice except creating it, itself!
     */
    AImageReader *reader_;
    bool recording_{false};
    Queuer *queuer_;

    /// FIXME
    sp<BufferItemConsumer> mBufferItemConsumer;
    struct FrameListener;
    sp<FrameListener> mFrameListener;

    // definition of handler and message
    enum {
        kWhatBufferRemoved,
        kWhatImageAvailable,
    };
    static const char* kCallbackFpKey;
    static const char* kContextKey;
    static const char* kGraphicBufferKey;

    class CallbackHandler : public AHandler {
    public:
        CallbackHandler(ImageReader *reader) : mReader(reader) {}

        void onMessageReceived(const sp<AMessage> &msg) override;

    private:
        ImageReader *mReader;
    };

    sp<CallbackHandler> mHandler;
    sp<ALooper> mCbLooper;

    struct FrameListener : public ConsumerBase::FrameAvailableListener { // FIXME
    public:
        explicit FrameListener(ImageReader *parent) : mReader(parent) {}

        void onFrameAvailable(const BufferItem &item) override;

        media_status_t setImageListener(ImageReader_ImageListener *listener);

    private:
        ImageReader_ImageListener mListener = {nullptr, nullptr};
        const wp<ImageReader> mReader;
        Mutex mLock;
    };

    struct BufferRemovedListener : public BufferItemConsumer::BufferFreedListener {
    public:
        explicit BufferRemovedListener(ImageReader* parent) : mReader(parent) {}

        void onBufferFreed(const wp<GraphicBuffer>& graphicBuffer) override;

        media_status_t setBufferRemovedListener(ImageReader_BufferRemovedListener* listener);

    private:
        ImageReader_BufferRemovedListener mListener = {nullptr, nullptr};
        const wp<ImageReader>     mReader;
        Mutex                      mLock;
    };
    sp<BufferRemovedListener> mBufferRemovedListener;
};

#endif  // VIS_IMAGE_READER_H

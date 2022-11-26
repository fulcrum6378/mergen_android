#ifndef VIS_IMAGE_READER_H
#define VIS_IMAGE_READER_H

#include <functional>
#include <media/NdkImageReader.h>

struct ImageFormat {
    int32_t width;
    int32_t height;
    int32_t format;
};

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

class ImageReader {
public:
    // Ctor and Dtor()
    explicit ImageReader(ImageFormat *format);

    ~ImageReader();

    void SetMirrorWindow(ANativeWindow *window);

    // Report cached ANativeWindow, which was used to create camera's capture session output.
    ANativeWindow *GetNativeWindow(void);

    /**
     * Retrieve the next image in ImageReader's bufferQueue, NOT the last image so
     * no image is skipped. Recommended for batch/background processing.
     */
    AImage *GetNextImage(void);

    /**
     * Retrieve the last image in ImageReader's bufferQueue, deleting images in
     * in front of it on the queue. Recommended for real-time processing.
     */
    AImage *GetLatestImage(void);

    // static void DeleteImage(AImage *image);

    // called by AImageReader when a frame is captured
    void ImageCallback(AImageReader *reader);

    /**
     * Present camera image to the given display buffer. Avaliable image is converted
     *   to display buffer format. Supported display format:
     *      WINDOW_FORMAT_RGBX_8888
     *      WINDOW_FORMAT_RGBA_8888
     *   @param buf {@link ANativeWindow_Buffer} for image to display to.
     *   @param image a {@link AImage} instance, source of image conversion.
     *            it will be deleted via {@link AImage_delete}
     *   @return true on success, false on failure
     *   https://mathbits.com/MathBits/TISection/Geometry/Transformations2.htm
     */
    static bool Mirror(ANativeWindow_Buffer *buf, AImage *image);

    /**
     * regsiter a callback function for client to be notified that jpeg already written out.
     * @param ctx is client context when callback is invoked
     * @param callback is the actual callback function
     */
    void RegisterCallback(void *ctx, std::function<void(void *ctx, const char *fileName)>);

    void WriteFile(AImage *image);

private:
    AImageReader *reader_;
    ANativeWindow *mirror_;

    std::function<void(void *ctx, const char *fileName)> callback_;
    void *callbackCtx_;
};

#endif  // VIS_IMAGE_READER_H

#ifndef VIS_IMAGE_READER_H
#define VIS_IMAGE_READER_H

#include <functional>
#include <media/NdkImageReader.h>

struct ImageFormat {
    int32_t width;
    int32_t height;
    int32_t format;
};

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

class ImageReader {
public:
    explicit ImageReader(ImageFormat *format);

    ~ImageReader();

    void SetMirrorWindow(ANativeWindow *window);

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

    bool SetRecording(bool b);

    void WriteFile(AImage *image) const;

private:
    AImageReader *reader_;
    ANativeWindow *mirror_;
    bool recording_{false};
    int64_t count_{0};
};

#endif  // VIS_IMAGE_READER_H

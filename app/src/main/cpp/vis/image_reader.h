#ifndef VIS_IMAGE_READER_H
#define VIS_IMAGE_READER_H

#include <media/NdkImageReader.h>
#include <utility>

#include "../global.h"

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
    explicit ImageReader(std::pair<int32_t, int32_t> *dimen);

    ~ImageReader();

    void SetMirrorWindow(ANativeWindow *window);

    ANativeWindow *GetNativeWindow(void);

    // called when a frame is captured
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

    static void WriteFile(AImage *image, int64_t i);

private:
    AImageReader *reader_;
    ANativeWindow *mirror_;
    bool recording_{false};
    int64_t count_{1};
};

#endif  // VIS_IMAGE_READER_H

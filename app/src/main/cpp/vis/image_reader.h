#ifndef VIS_IMAGE_READER_H
#define VIS_IMAGE_READER_H

#include <media/NdkImage.h>
#include <media/NdkImageReader.h>
#include <utility>

#include "../mem/queuer.h"

// together with AIMAGE_FORMAT_JPEG, these are the only supported options for my phone apparently!
#define VIS_IMAGE_FORMAT AIMAGE_FORMAT_YUV_420_888

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
    explicit ImageReader(std::pair<int32_t, int32_t> *dimen, Queuer *queuer);

    void SetMirrorWindow(ANativeWindow *window);

    ANativeWindow *GetNativeWindow(void);

    void ImageCallback(AImageReader *reader);

    static bool Mirror(ANativeWindow_Buffer *buf, AImage *image);

    bool SetRecording(bool b);

    static void WriteFile(AImage *image, int64_t i);

    ~ImageReader();

private:
    AImageReader *reader_;
    ANativeWindow *mirror_;
    bool recording_{false};
    int64_t count_{1};
    Queuer *queuer_;
};

#endif  // VIS_IMAGE_READER_H

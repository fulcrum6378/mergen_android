#ifndef VIS_IMAGE_READER_H
#define VIS_IMAGE_READER_H

#include <functional>
#include <media/NdkImageReader.h>

struct ImageFormat {
    int32_t width;
    int32_t height;
    int32_t format;  // The format is fixed to YUV_420 format
};

class ImageReader {
public:
    // Ctor and Dtor()
    explicit ImageReader(ImageFormat *format);

    ~ImageReader();

    // Report cached ANativeWindow, which was used to create camera's capture session output.
    ANativeWindow *GetNativeWindow(void);

    // Retrieve Image on the top of Reader's queue
    AImage *GetNextImage(void);

    // Retrieve Image on the back of Reader's queue, dropping older images
    AImage *GetLatestImage(void);

    /**
     * Delete Image
     * @param image {@link AImage} instance to be deleted
     */
    void DeleteImage(AImage *image);

    /**
     * AImageReader callback handler. Called by AImageReader when a frame is
     * captured
     * (Internal function, not to be called by clients)
     */
    void ImageCallback(AImageReader *reader);

    /**
     * DisplayImage()
     *   Present camera image to the given display buffer. Avaliable image is
     * converted
     *   to display buffer format. Supported display format:
     *      WINDOW_FORMAT_RGBX_8888
     *      WINDOW_FORMAT_RGBA_8888
     *   @param buf {@link ANativeWindow_Buffer} for image to display to.
     *   @param image a {@link AImage} instance, source of image conversion.
     *            it will be deleted via {@link AImage_delete}
     *   @return true on success, false on failure
     */
    bool DisplayImage(ANativeWindow_Buffer *buf, AImage *image);

    /**
     * Configure the rotation angle necessary to apply to
     * Camera image when presenting: all rotations should be accumulated:
     *    CameraSensorOrientation + Android Device Native Orientation +
     *    Human Rotation (rotated degree related to Phone native orientation
     */
    void SetPresentRotation(int32_t angle);

    /**
     * regsiter a callback function for client to be notified that jpeg already
     * written out.
     * @param ctx is client context when callback is invoked
     * @param callback is the actual callback function
     */
    void RegisterCallback(void *ctx, std::function<void(void *ctx, const char *fileName)>);

    void WriteFile(AImage *image);

private:
    int32_t presentRotation_;
    AImageReader *reader_;

    std::function<void(void *ctx, const char *fileName)> callback_;
    void *callbackCtx_;

    void PresentImage(ANativeWindow_Buffer *buf, AImage *image);

    void PresentImage90(ANativeWindow_Buffer *buf, AImage *image);

    void PresentImage180(ANativeWindow_Buffer *buf, AImage *image);

    void PresentImage270(ANativeWindow_Buffer *buf, AImage *image);
};

#endif  // VIS_IMAGE_READER_H

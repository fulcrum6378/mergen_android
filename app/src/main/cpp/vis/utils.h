#ifndef VIS_UTILS_H
#define VIS_UTILS_H

#include <camera/NdkCameraDevice.h>
#include <camera/NdkCameraManager.h>
#include <camera/NdkCameraMetadataTags.h>

/*
 * A set of macros to call into Camera APIs. The API is grouped with a few
 * objects, with object name as the prefix of function names.
 */
#define CALL_CAMERA(func) { camera_status_t status = func; }
#define CALL_MGR(func) CALL_CAMERA(ACameraManager_##func)
#define CALL_DEV(func) CALL_CAMERA(ACameraDevice_##func)
#define CALL_METADATA(func) CALL_CAMERA(ACameraMetadata_##func)
#define CALL_CONTAINER(func) CALL_CAMERA(ACaptureSessionOutputContainer_##func)
#define CALL_OUTPUT(func) CALL_CAMERA(ACaptureSessionOutput_##func)
#define CALL_TARGET(func) CALL_CAMERA(ACameraOutputTarget_##func)
#define CALL_REQUEST(func) CALL_CAMERA(ACaptureRequest_##func)
#define CALL_SESSION(func) CALL_CAMERA(ACameraCaptureSession_##func)

struct ImageFormat {
    int32_t width;
    int32_t height;
    int32_t format;
};

#endif //VIS_UTILS_H

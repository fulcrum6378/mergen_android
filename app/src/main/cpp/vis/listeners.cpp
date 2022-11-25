#include <camera/NdkCameraManager.h>
#include <cinttypes>
#include <queue>
#include <thread>
#include <utility>

#include "ndk_camera.h"
#include "utils.h"
#include "../otr/debug.h"

ACameraManager_AvailabilityCallbacks *NDKCamera::GetManagerListener() {
    static ACameraManager_AvailabilityCallbacks cameraMgrListener = {
            .context = this,
            .onCameraAvailable = nullptr,
            .onCameraUnavailable = nullptr,
    };
    return &cameraMgrListener;
}

void OnDeviceStateChanges(void *ctx, ACameraDevice *dev) {
    reinterpret_cast<NDKCamera *>(ctx)->OnDeviceState(dev);
}

void OnDeviceErrorChanges(void *ctx, ACameraDevice *dev, int err) {
    reinterpret_cast<NDKCamera *>(ctx)->OnDeviceError(dev, err);
}

ACameraDevice_stateCallbacks *NDKCamera::GetDeviceListener() {
    static ACameraDevice_stateCallbacks cameraDeviceListener = {
            .context = this,
            .onDisconnected = ::OnDeviceStateChanges,
            .onError = ::OnDeviceErrorChanges,
    };
    return &cameraDeviceListener;
}

void NDKCamera::OnDeviceState(ACameraDevice *dev) {
    std::string id(ACameraDevice_getId(dev));
    LOGW("device %s is disconnected", id.c_str());

    ACameraDevice_close(cameras_[id].device_);
    cameras_.erase(id);
}

void NDKCamera::OnDeviceError(ACameraDevice */*dev*/, int /*err*/) {
}

// CaptureSession state callbacks
void OnSessionClosed(void *ctx, ACameraCaptureSession *ses) {
    LOGW("session %p closed", ses);
    reinterpret_cast<NDKCamera *>(ctx)->OnSessionState(ses, CaptureSessionState::CLOSED);
}

void OnSessionReady(void *ctx, ACameraCaptureSession *ses) {
    LOGW("session %p ready", ses);
    reinterpret_cast<NDKCamera *>(ctx)->OnSessionState(ses, CaptureSessionState::READY);
}

void OnSessionActive(void *ctx, ACameraCaptureSession *ses) {
    LOGW("session %p active", ses);
    reinterpret_cast<NDKCamera *>(ctx)->OnSessionState(ses, CaptureSessionState::ACTIVE);
}

ACameraCaptureSession_stateCallbacks *NDKCamera::GetSessionListener() {
    static ACameraCaptureSession_stateCallbacks sessionListener = {
            .context = this,
            .onClosed = ::OnSessionClosed,
            .onReady = ::OnSessionReady,
            .onActive = ::OnSessionActive,
    };
    return &sessionListener;
}

/**
 * Handles capture session state changes.
 *   Update into internal session state.
 */
void NDKCamera::OnSessionState(ACameraCaptureSession *ses, CaptureSessionState state) {
    if (!ses || ses != captureSession_) {
        LOGW("CaptureSession is %s", (ses ? "NOT our session" : "NULL"));
        return;
    }
    ASSERT(state < CaptureSessionState::MAX_STATE, "Wrong state %d", state)
    captureSessionState_ = state;
}

void SessionCaptureCallback_OnStarted(
        void *context, ACameraCaptureSession *session,
        const ACaptureRequest *request, int64_t timestamp) {
}

void SessionCaptureCallback_OnCompleted(
        void *context, ACameraCaptureSession *session, ACaptureRequest *request,
        const ACameraMetadata *result) {
    auto *ndkCamera = static_cast<NDKCamera *>(context);
    /*ANativeWindow *window = static_cast<NDKCamera *>(context)->window;
    ANativeWindow_Buffer outBuffer;
    ARect inOutDirtyBounds;
    //ANativeWindow_unlockAndPost(window);
    // [SurfaceTexture-0-2290-0](id:8f200000000,api:4,p:4292,c:2290) connect: already connected (cur=4 req=2)
    ANativeWindow_lock(window,&outBuffer, &inOutDirtyBounds);*/
    // FIXME WE FOUND THE CALLBACK FOR EACH FRAME BUT WE CANNOT RETRIEVE THE CAPTURED IMAGE!!!
    //   ImageReader still needs AImage *image
    //   ANativeWindow_Buffer is junk; AImage is what we want!
    //   Or perhaps we need both but the data is in AImage.
    AImage *fuck = ndkCamera->reader_->GetNextImage();
    int32_t w, h;
    AImage_getWidth(fuck, &w);
    AImage_getHeight(fuck, &h);
    LOGW("%s", ("SessionCaptureCallback_OnCompleted: " + std::to_string(w) +
                " x " + std::to_string(h)).c_str());
    /*SessionCaptureCallback_OnCompleted: 720 x 720
14:40:14.414  W  SessionCaptureCallback_OnCompleted: 720 x 720
14:40:14.474  W  SessionCaptureCallback_OnCompleted: 720 x 720
14:40:14.536  W  Unable to acquire a lockedBuffer, very likely client tries to lock more than maxImages buffers
14:40:14.536  W  SessionCaptureCallback_OnCompleted: 720 x 720
14:40:14.597  W  Unable to acquire a lockedBuffer, very likely client tries to lock more than maxImages buffers
14:40:14.597  E  AImage_getWidth: bad argument. image 0x0 width 0x7306a798f4*/
    ndkCamera->reader_->DeleteImage(fuck);
    // TODO WE DID IT... THO WE LOST PREVIEW SCREEN :(
}

/*void SessionCaptureCallback_OnFailed(
        void *context, ACameraCaptureSession *session, ACaptureRequest *request,
        ACameraCaptureFailure *failure) {
    std::thread captureFailedThread(
            &NDKCamera::OnCaptureFailed, static_cast<NDKCamera *>(context), session, request,
            failure);
    captureFailedThread.detach();
}*/

void SessionCaptureCallback_OnSequenceAborted(
        void *context, ACameraCaptureSession *session, int sequenceId) {
    /*std::thread sequenceThread(
            &NDKCamera::OnCaptureSequenceEnd, static_cast<NDKCamera *>(context), session,
            sequenceId, static_cast<int64_t>(-1));
    sequenceThread.detach();*/
}

void SessionCaptureCallback_OnSequenceEnd(
        void *context, ACameraCaptureSession *session, int sequenceId, int64_t frameNumber) {
    /*std::thread sequenceThread(
            &NDKCamera::OnCaptureSequenceEnd, static_cast<NDKCamera *>(context), session,
            sequenceId, frameNumber);
    sequenceThread.detach();*/
}

ACameraCaptureSession_captureCallbacks *NDKCamera::GetCaptureCallback() {
    static ACameraCaptureSession_captureCallbacks captureListener{
            .context = this,
            .onCaptureStarted = SessionCaptureCallback_OnStarted,
            .onCaptureProgressed = nullptr,
            .onCaptureCompleted = SessionCaptureCallback_OnCompleted,
            .onCaptureFailed = nullptr/*SessionCaptureCallback_OnFailed*/,
            .onCaptureSequenceCompleted = SessionCaptureCallback_OnSequenceEnd,
            .onCaptureSequenceAborted = SessionCaptureCallback_OnSequenceAborted,
            .onCaptureBufferLost = nullptr,
    };
    return &captureListener;
}

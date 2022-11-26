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


/*void SessionCaptureCallback_OnStarted(
        void *context, ACameraCaptureSession *session,
        const ACaptureRequest *request, int64_t timestamp) {
}*/

void SessionCaptureCallback_OnCompleted(
        void *context, ACameraCaptureSession *session, ACaptureRequest *request,
        const ACameraMetadata *result) {
    return;
    auto *ndkCamera = static_cast<NDKCamera *>(context);
    AImage *fuck = ndkCamera->reader->GetNextImage();
    int32_t w, h;
    AImage_getWidth(fuck, &w);
    AImage_getHeight(fuck, &h);
    LOGW("%s", ("SessionCaptureCallback_OnCompleted: " + std::to_string(w) +
                " x " + std::to_string(h)).c_str());
    std::thread writeFileHandler(&ImageReader::WriteFile, ndkCamera->reader, fuck);
    writeFileHandler.detach(); //ndkCamera->reader->DeleteImage(fuck);
}

/*void SessionCaptureCallback_OnFailed(
        void *context, ACameraCaptureSession *session, ACaptureRequest *request,
        ACameraCaptureFailure *failure) {
}*/

/*void SessionCaptureCallback_OnSequenceAborted(
        void *context, ACameraCaptureSession *session, int sequenceId) {
}*/

/*void SessionCaptureCallback_OnSequenceEnd(
        void *context, ACameraCaptureSession *session, int sequenceId, int64_t frameNumber) {
}*/

ACameraCaptureSession_captureCallbacks *NDKCamera::GetCaptureCallback() {
    static ACameraCaptureSession_captureCallbacks captureListener{
            .context = this,
            .onCaptureStarted = nullptr/*SessionCaptureCallback_OnStarted*/,
            .onCaptureProgressed = nullptr,
            .onCaptureCompleted = SessionCaptureCallback_OnCompleted,
            .onCaptureFailed = nullptr/*SessionCaptureCallback_OnFailed*/,
            .onCaptureSequenceCompleted = nullptr/*SessionCaptureCallback_OnSequenceEnd*/,
            .onCaptureSequenceAborted = nullptr/*SessionCaptureCallback_OnSequenceAborted*/,
            .onCaptureBufferLost = nullptr,
    };
    return &captureListener;
}

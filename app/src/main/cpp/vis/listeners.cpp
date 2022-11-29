#include "camera.h"
#include "../global.h"

ACameraManager_AvailabilityCallbacks *Camera::GetManagerListener() {
    static ACameraManager_AvailabilityCallbacks cameraMgrListener = {
            .context = this,
            .onCameraAvailable = nullptr,
            .onCameraUnavailable = nullptr,
    };
    return &cameraMgrListener;
}

void OnDeviceStateChanges(void *ctx, ACameraDevice *dev) {
    reinterpret_cast<Camera *>(ctx)->OnDeviceState(dev);
}

void OnDeviceErrorChanges(void *ctx, ACameraDevice *dev, int err) {
    reinterpret_cast<Camera *>(ctx)->OnDeviceError(dev, err);
}

ACameraDevice_stateCallbacks *Camera::GetDeviceListener() {
    static ACameraDevice_stateCallbacks cameraDeviceListener = {
            .context = this,
            .onDisconnected = ::OnDeviceStateChanges,
            .onError = ::OnDeviceErrorChanges,
    };
    return &cameraDeviceListener;
}

void Camera::OnDeviceState(ACameraDevice *dev) {
    std::string id(ACameraDevice_getId(dev));
    LOGW("device %s is disconnected", id.c_str());

    ACameraDevice_close(cameras_[id].device_);
    cameras_.erase(id);
}

void Camera::OnDeviceError(ACameraDevice */*dev*/, int /*err*/) {
}

// CaptureSession state callbacks
void OnSessionClosed(void *ctx, ACameraCaptureSession *ses) {
    LOGW("session %p closed", ses);
    reinterpret_cast<Camera *>(ctx)->OnSessionState(ses, CaptureSessionState::CLOSED);
}

void OnSessionReady(void *ctx, ACameraCaptureSession *ses) {
    LOGW("session %p ready", ses);
    reinterpret_cast<Camera *>(ctx)->OnSessionState(ses, CaptureSessionState::READY);
}

void OnSessionActive(void *ctx, ACameraCaptureSession *ses) {
    LOGW("session %p active", ses);
    reinterpret_cast<Camera *>(ctx)->OnSessionState(ses, CaptureSessionState::ACTIVE);
}

ACameraCaptureSession_stateCallbacks *Camera::GetSessionListener() {
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
void Camera::OnSessionState(ACameraCaptureSession *ses, CaptureSessionState state) {
    if (!ses || ses != captureSession_) {
        LOGW("CaptureSession is %s", (ses ? "NOT our session" : "NULL"));
        return;
    }
    ASSERT(state < CaptureSessionState::MAX_STATE, "Wrong state %d", state)
    captureSessionState_ = state;
}

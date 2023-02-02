#include <android/asset_manager_jni.h>
#include <android/native_window_jni.h>
#include <game-activity/native_app_glue/android_native_app_glue.h>
#include <cmath>
#include <cstdio>
#include <cstdlib>

#include <iostream>
#include "vulkan_test.h"

struct VulkanEngine {
    struct android_app *app{};
    vkt::HelloVK *app_backend{};
    bool canRender = false;
};

static void HandleCmd(struct android_app *app, int32_t cmd) {
    auto *engine = (VulkanEngine *) app->userData;
    switch (cmd) {
        case APP_CMD_START:
            if (engine->app->window != nullptr) {
                engine->app_backend->reset(app->window, app->activity->assetManager);
                engine->app_backend->initVulkan();
                engine->canRender = true;
            }
        case APP_CMD_INIT_WINDOW:
            // The window is being shown, get it ready.
            LOGI("Called - APP_CMD_INIT_WINDOW");
            if (engine->app->window != nullptr) {
                LOGI("Setting a new surface");
                engine->app_backend->reset(app->window, app->activity->assetManager);
                if (!engine->app_backend->initialized) {
                    LOGI("Starting application");
                    engine->app_backend->initVulkan();
                }
                engine->canRender = true;
            }
            break;
        case APP_CMD_TERM_WINDOW:
            // The window is being hidden or closed, clean it up.
            engine->canRender = false;
            break;
        case APP_CMD_DESTROY:
            // The window is being hidden or closed, clean it up.
            LOGI("Destroying");
            engine->app_backend->cleanup();
        default:
            break;
    }
}

extern "C" bool VulkanKeyEventFilter(const GameActivityKeyEvent *) {
    return false;
}
extern "C" bool VulkanMotionEventFilter(const GameActivityMotionEvent *) {
    return false;
}

static void HandleInputEvents(struct android_app *app) {
    auto inputBuf = android_app_swap_input_buffers(app);
    if (inputBuf == nullptr) {
        return;
    }

    // For the minimum, apps need to process the exit event (for example,
    // listening to AKEYCODE_BACK). This sample has done that in the Kotlin side
    // and not processing other input events, we just reset the event counter
    // inside the android_input_buffer to keep app glue code in a working state.
    android_app_clear_motion_events(inputBuf);
    android_app_clear_motion_events(inputBuf);
}

/*static int32_t engine_handle_input(struct android_app *app, AInputEvent *event) {
    auto* engine = (struct engine*)app->userData;
    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
        engine->animating = 1;
        engine->state.x = AMotionEvent_getX(event, 0);
        engine->state.y = AMotionEvent_getY(event, 0);
        return 1;
    }
    return 0;
}*/

/*static void engine_handle_cmd(struct android_app* app, int32_t cmd) {
    auto* engine = (struct engine*)app->userData;
    switch (cmd) {
        case APP_CMD_SAVE_STATE:
            // The system has asked us to save our current state.  Do so.
            engine->app->savedState = malloc(sizeof(struct saved_state));
            *((struct saved_state*)engine->app->savedState) = engine->state;
            engine->app->savedStateSize = sizeof(struct saved_state);
            break;
        case APP_CMD_INIT_WINDOW:
            // The window is being shown, get it ready.
            if (engine->app->window != nullptr) {
                engine_init_display(engine);
                engine_draw_frame(engine);
            }
            break;
        case APP_CMD_TERM_WINDOW:
            // The window is being hidden or closed, clean it up.
            //engine_term_display(engine);
            break;
        case APP_CMD_GAINED_FOCUS:
            break;
        case APP_CMD_LOST_FOCUS:
            // Also stop animating.
            engine->animating = 0;
            engine_draw_frame(engine);
            break;
        default:
            break;
    }
}*/

_Noreturn void android_main(struct android_app *state) {
    /*app->onAppCmd = engine_handle_cmd;
    app->onInputEvent = engine_handle_input;*/

    VulkanEngine engine{};
    vkt::HelloVK vulkanBackend{};

    engine.app = state;
    engine.app_backend = &vulkanBackend;
    state->userData = &engine;
    state->onAppCmd = HandleCmd;

    android_app_set_key_event_filter(state, VulkanKeyEventFilter);
    android_app_set_motion_event_filter(state, VulkanMotionEventFilter);

    while (true) {
        int ident;
        int events;
        android_poll_source *source;
        while ((ident = ALooper_pollAll(engine.canRender ? 0 : -1, nullptr, &events,
                                        (void **) &source)) >= 0) {
            if (source != nullptr) source->process(state, source);
        }

        HandleInputEvents(state);

        engine.app_backend->render();
    }
}

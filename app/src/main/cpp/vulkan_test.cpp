#include <android_native_app_glue.h>

#include "vulkan_test.h"

struct VulkanEngine {
    struct android_app *app{};
    HelloVK *app_backend{};
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
            engine->canRender = false;
            break;
        case APP_CMD_DESTROY:
            LOGI("Destroying");
            engine->app_backend->cleanup();
        default:
            break;
    }
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"

_Noreturn void android_main(struct android_app *state) {
    VulkanEngine engine{};
    HelloVK vulkanBackend{};

    engine.app = state;
    engine.app_backend = &vulkanBackend;
    state->userData = &engine;
    state->onAppCmd = HandleCmd;

    while (true) {
        int events;
        android_poll_source *source;
        while (ALooper_pollAll(engine.canRender ? 0 : -1,
                               nullptr, &events, (void **) &source) >= 0)
            if (source != nullptr)
                source->process(state, source);
        engine.app_backend->render();
    }
}

#pragma clang diagnostic pop

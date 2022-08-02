#include <android_native_app_glue.h>

// Saved state data
struct saved_state {
};

// Shared state for the app
struct engine {
    struct android_app *app;
    struct saved_state state;
};

static int32_t engine_handle_input(struct android_app *app, AInputEvent *event) {
    // auto *engine = (struct engine *) app->userData;
    return 0;
}

static void engine_handle_cmd(struct android_app *app, int32_t cmd) {
    // auto *engine = (struct engine *) app->userData;
}

void android_main(struct android_app *state) {
    struct engine engine{};
    state->userData = &engine;
    state->onAppCmd = engine_handle_cmd;
    state->onInputEvent = engine_handle_input;
    engine.app = state;
}

extern "C" JNIEXPORT void Java_ir_mahdiparastesh_mergen_Main_preview(
        JNIEnv *env, jobject /*main*/) {
}

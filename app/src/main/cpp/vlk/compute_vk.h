#ifndef VLK_COMPUTE_VK_H
#define VLK_COMPUTE_VK_H

#include <android/native_window.h>

#include "../global.h"
#include "global.h"

class ComputeVK {
public:
    void initVulkan();

    void render();

    void cleanup();

    void reset(ANativeWindow *newWindow, AAssetManager *newManager);

    bool initialized = false;
private:
};

#endif //VLK_COMPUTE_VK_H

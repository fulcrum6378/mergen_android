#ifndef VLK_GLOBAL_H
#define VLK_GLOBAL_H

#include <android/asset_manager.h>
#include <vulkan/vulkan.h>

#include <vector>

#define VK_CHECK(x)                           \
  do {                                        \
    VkResult err = x;                         \
    if (err) {                                \
      LOGE("Detected Vulkan error: %d", err); \
      abort();                                \
    }                                         \
  } while (0)

#endif //VLK_GLOBAL_H

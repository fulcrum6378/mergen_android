#ifndef GLOBAL_VK_H
#define GLOBAL_VK_H

#include <android/asset_manager.h>
#include <cassert>
#include <vector>
#include <vulkan/vulkan.h>

#include "global.hpp"

/**
 * You need to download the latest binaries in
 * <a href="https://github.com/KhronosGroup/Vulkan-ValidationLayers/releases">
 * Vulkan-ValidationLayers</a> into `android/jniLibs` (`app/src/main/jniLibs`),
 * in order to be able to change this value to "true".
 */
#define ENABLE_VALIDATION_LAYERS true

/** Global struct for creating Vulkan instances. */
inline static VkApplicationInfo vkAppInfo{
        VK_STRUCTURE_TYPE_APPLICATION_INFO, nullptr,
        "Mergen", VK_MAKE_VERSION(0, 4, 0),
        "No Engine", VK_MAKE_VERSION(1, 0, 0),
        VK_API_VERSION_1_0
};

#define VK_CHECK(x)                           \
  do {                                        \
    VkResult err = x;                         \
    if (err) {                                \
      LOGE("Detected Vulkan error: %d", err); \
      abort();                                \
    }                                         \
  } while (0)

static std::vector<uint8_t> LoadShaderCode(
        const char *file_path, AAssetManager *assetManager) {
    std::vector<uint8_t> file_content;
    assert(assetManager);
    AAsset *file =
            AAssetManager_open(assetManager, file_path, AASSET_MODE_BUFFER);
    size_t file_length = AAsset_getLength(file);

    file_content.resize(file_length);

    AAsset_read(file, file_content.data(), file_length);
    AAsset_close(file);
    return file_content;
}

#if ENABLE_VALIDATION_LAYERS

static android_LogPriority toStringMessageSeverity(VkDebugUtilsMessageSeverityFlagBitsEXT s) {
    switch (s) {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            return ANDROID_LOG_VERBOSE;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            return ANDROID_LOG_INFO;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            return ANDROID_LOG_WARN;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            return ANDROID_LOG_ERROR;
        default: // impossible
            return ANDROID_LOG_UNKNOWN;
    }
}

static const char *toStringMessageType(VkDebugUtilsMessageTypeFlagsEXT s) {
    if (s == (VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
              VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
              VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT))
        return "General | Validation | Performance";
    if (s == (VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
              VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT))
        return "Validation | Performance";
    if (s == (VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
              VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT))
        return "General | Performance";
    if (s == (VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT))
        return "Performance";
    if (s == (VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
              VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT))
        return "General | Validation";
    if (s == VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) return "Validation";
    if (s == VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT) return "General";
    return "IMPOSSIBLE";
}

static VKAPI_ATTR VkBool32 VKAPI_CALL
debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
              VkDebugUtilsMessageTypeFlagsEXT messageType,
              const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
              void * /*pUserData*/) {
    __android_log_print(
            toStringMessageSeverity(messageSeverity), LOG_TAG,
            "VKAPI [%s]: %s", toStringMessageType(messageType), pCallbackData->pMessage);

    return VK_FALSE; // means "do not abort the Vulkan call and continue"
}

static void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo) {
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
}

static VkResult CreateDebugUtilsMessengerEXT(
        VkInstance instance,
        const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
        const VkAllocationCallbacks *pAllocator,
        VkDebugUtilsMessengerEXT *pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)
            vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr)
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    else return VK_ERROR_EXTENSION_NOT_PRESENT;
}

static void DestroyDebugUtilsMessengerEXT(
        VkInstance instance,
        VkDebugUtilsMessengerEXT debugMessenger,
        const VkAllocationCallbacks *pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)
            vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) func(instance, debugMessenger, pAllocator);
}

#endif //ENABLE_VALIDATION_LAYERS

#endif //GLOBAL_VK_H

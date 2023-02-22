#include <cassert>

#include "compute_vk.h"

#pragma clang diagnostic push
#pragma ide diagnostic ignored "ConstantConditionsOC"

// PUBLIC:

void ComputeVK::run() {
    // Initilising Vulkan
    createInstance();
    setupDebugMessenger();
    pickPhysicalDevice();
    createLogicalDeviceAndQueue();
    createBuffer();
    createDescriptorSetLayout();
    createDescriptorSet();
    createComputePipeline();
    createCommandBuffer();

    runCommandBuffer();
    saveRenderedImage();

    cleanup();
}

// PRIVATE (INIT):

void ComputeVK::createInstance() {
    assert(!enableValidationLayers || checkValidationLayerSupport());
    auto requiredExtensions = getRequiredExtensions(enableValidationLayers);

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Mergen";
    appInfo.applicationVersion = VK_MAKE_VERSION(0, 2, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = (uint32_t) requiredExtensions.size();
    createInfo.ppEnabledExtensionNames = requiredExtensions.data();

    if (enableValidationLayers) {
        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
        populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *) &debugCreateInfo;
    } else {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;
    }
    VK_CHECK(vkCreateInstance(&createInfo, nullptr, &instance));
}

void ComputeVK::setupDebugMessenger() {
    if (!enableValidationLayers) return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    populateDebugMessengerCreateInfo(createInfo);

    VK_CHECK(CreateDebugUtilsMessengerEXT(
            instance, &createInfo, nullptr, &debugMessenger));
}

void ComputeVK::pickPhysicalDevice() {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    assert(deviceCount > 0);
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(
            instance, &deviceCount, devices.data());
    VkPhysicalDevice &first = devices[0];
    physicalDevice = first;
    assert(physicalDevice != VK_NULL_HANDLE);
}

void ComputeVK::createLogicalDeviceAndQueue() {
    VkDeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueFamilyIndex = getComputeQueueFamilyIndex(); // find queue family with compute capability.
    queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
    queueCreateInfo.queueCount = 1; // create one queue in this family. We don't need more.
    float queuePriorities = 1.0;  // we only have one queue, so this is not that imporant.
    queueCreateInfo.pQueuePriorities = &queuePriorities;
}

void ComputeVK::createBuffer() {
}

void ComputeVK::createDescriptorSetLayout() {
}

void ComputeVK::createDescriptorSet() {
}

void ComputeVK::createComputePipeline() {
}

void ComputeVK::createCommandBuffer() {
}

void ComputeVK::runCommandBuffer() {
}

void ComputeVK::saveRenderedImage() {
}

void ComputeVK::cleanup() {
    vkDestroyDevice(device, nullptr);
    if (enableValidationLayers)
        DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    vkDestroyInstance(instance, nullptr);
}

// PRIVATE (MISC):

std::vector<const char *> ComputeVK::getRequiredExtensions(bool _enableValidationLayers) {
    std::vector<const char *> extensions;
    if (_enableValidationLayers) extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    return extensions;
}

bool ComputeVK::checkValidationLayerSupport() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char *layerName: validationLayers) {
        bool layerFound = false;
        for (const auto &layerProperties: availableLayers)
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        if (!layerFound) return false;
    }
    return true;
}

uint32_t ComputeVK::getComputeQueueFamilyIndex() {
    uint32_t queueFamilyCount;
    vkGetPhysicalDeviceQueueFamilyProperties(
            physicalDevice, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(
            physicalDevice, &queueFamilyCount,
            queueFamilies.data());

    uint32_t i = 0;
    for (; i < queueFamilies.size(); ++i) {
        VkQueueFamilyProperties props = queueFamilies[i];
        if (props.queueCount > 0 && (props.queueFlags & VK_QUEUE_COMPUTE_BIT))
            // found a queue with compute. We're done!
            break;
    }
    ASSERT(i != queueFamilies.size(), "Could not find a queue family that supports operations");
    return i;
}

// STATIC:

#pragma clang diagnostic pop

#ifndef VLK_HELLO_VK_H
#define VLK_HELLO_VK_H

#include <android/native_window.h>

#include <array>
#include <optional>

#include "global.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-pragmas"
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
#pragma ide diagnostic ignored "OCUnusedStructInspection"

const int MAX_FRAMES_IN_FLIGHT = 2;

struct UniformBufferObject {
    std::array<float, 16> mvp;
};

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

struct ANativeWindowDeleter {
    void operator()(ANativeWindow *window) { ANativeWindow_release(window); }
};

/** @see <a href="https://github.com/android/ndk-samples/tree/main/hello-vulkan">Hello VK</a> */
class HelloVK {
public:
    void initVulkan();

    void render();

    void cleanup();

    void cleanupSwapChain();

    void reset(ANativeWindow *newWindow, AAssetManager *newManager);

    bool initialized = false;

private:
    void createInstance();

    void setupDebugMessenger();

    void createSurface();

    void pickPhysicalDevice();

    void createLogicalDeviceAndQueue();

    void establishDisplaySizeIdentity();

    void createSwapChain();

    void createImageViews();

    void createRenderPass();

    void createDescriptorSetLayout();

    void createUniformBuffers();

    void createDescriptorPool();

    void createDescriptorSets();

    void createGraphicsPipeline();

    void createFramebuffers();

    void createCommandPool();

    void createCommandBuffer();

    void createSyncObjects();


    static std::vector<const char *> getRequiredExtensions(bool _enableValidationLayers);

    bool checkValidationLayerSupport();

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice dev) const;

    bool checkDeviceExtensionSupport(VkPhysicalDevice dev);

    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice dev) const;

    bool isDeviceSuitable(VkPhysicalDevice dev);

    // VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);

    VkShaderModule createShaderModule(const std::vector<uint8_t> &code);

    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

    void recreateSwapChain();

    void onOrientationChange();

    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                      VkMemoryPropertyFlags properties, VkBuffer &buffer,
                      VkDeviceMemory &bufferMemory);

    void updateUniformBuffer(uint32_t currentImage);

    /** You need to download the latest binaries in
     * <a href="https://github.com/KhronosGroup/Vulkan-ValidationLayers/releases">
     * Vulkan-ValidationLayers</a>in order to be able to change this value to "true". */
    bool enableValidationLayers = true;

    const std::vector<const char *> validationLayers = {"VK_LAYER_KHRONOS_validation"};
    const std::vector<const char *> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    std::unique_ptr<ANativeWindow, ANativeWindowDeleter> window;
    AAssetManager *assetManager;

    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;

    VkSurfaceKHR surface;

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device;

    VkSwapchainKHR swapChain;
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    VkExtent2D displaySizeIdentity;
    std::vector<VkImageView> swapChainImageViews;
    std::vector<VkFramebuffer> swapChainFramebuffers;
    VkCommandPool commandPool;
    std::vector<VkCommandBuffer> commandBuffers;

    VkQueue graphicsQueue;
    VkQueue presentQueue;

    VkRenderPass renderPass;
    VkDescriptorSetLayout descriptorSetLayout;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;

    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;

    uint32_t currentFrame = 0;
    bool orientationChanged = false;
    VkSurfaceTransformFlagBitsKHR pretransformFlag;


    static void getPrerotationMatrix(const VkSurfaceCapabilitiesKHR &capabilities,
                                     const VkSurfaceTransformFlagBitsKHR &pretransformFlag,
                                     std::array<float, 16> &mat);
};

#pragma clang diagnostic pop

#endif //VLK_HELLO_VK_H
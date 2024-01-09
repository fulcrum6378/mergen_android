#ifndef VIS_ANALYSES_H
#define VIS_ANALYSES_H

#include <android/native_window.h>
#include <array>
#include <glm/glm.hpp>
#include <memory> // unique_ptr
#include <optional>

#include "../global_vk.hpp"

/** You need to download the latest binaries in
 * <a href="https://github.com/KhronosGroup/Vulkan-ValidationLayers/releases">
 * Vulkan-ValidationLayers</a> into `android/jniLibs` (`app/src/main/jniLibs`),
 * in order to be able to change this value to "true". */
#define ENABLE_VALIDATION_LAYERS true

static const int MAX_FRAMES_IN_FLIGHT = 2;

// TODO edit this to put data in it
struct UniformBufferObject {
    std::array<float, 16> mvp;
};

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    [[nodiscard]] bool isComplete() const {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

class Analyses {
public:
    Analyses(ANativeWindow *window, AAssetManager *assets);

    void render();

    ~Analyses();

private:
    void createInstance();

    std::vector<const char *> getRequiredExtensions();

#if ENABLE_VALIDATION_LAYERS

    bool checkValidationLayerSupport();

    void setupDebugMessenger();

#endif

    void createSurface();

    void pickPhysicalDevice();

    bool isDeviceSuitable(VkPhysicalDevice dev);

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice dev) const;

    bool checkDeviceExtensionSupport(VkPhysicalDevice dev);

    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice dev) const;

    void createLogicalDeviceAndQueue();

    void establishDisplaySizeIdentity();

    void createSwapChain();

    void createImageViews();

    void createRenderPass();

    void createDescriptorSetLayout();

    void createUniformBuffers();

    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                      VkMemoryPropertyFlags properties, VkBuffer &buffer,
                      VkDeviceMemory &bufferMemory);

    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    void createDescriptorPool();

    void createDescriptorSets();

    void createGraphicsPipeline(AAssetManager *assets);

    VkShaderModule createShaderModule(const std::vector<uint8_t> &code);

    void createFramebuffers();

    void createCommandPool();

    void createCommandBuffer();

    void createSyncObjects();

    // RENDER()

    void recreateSwapChain();

    void cleanupSwapChain();

    void updateUniformBuffer(uint32_t currentImage);

    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);


    ANativeWindow *window_;
    const std::vector<const char *> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    VkInstance instance;
    VkSurfaceKHR surface;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device;

#if ENABLE_VALIDATION_LAYERS
    const std::vector<const char *> validationLayers = {"VK_LAYER_KHRONOS_validation"};
    VkDebugUtilsMessengerEXT debugMessenger;
#endif

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
    VkSurfaceTransformFlagBitsKHR pretransformFlag;
};

#endif //VIS_ANALYSES_H
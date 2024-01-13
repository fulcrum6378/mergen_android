#ifndef VIS_ANALYSES_H
#define VIS_ANALYSES_H

#include <android/native_window.h>
#include <array>
#include <glm/glm.hpp>
#include <memory> // unique_ptr
#include <optional>

#include "../global_vk.hpp"

#define MAX_FRAMES_IN_FLIGHT 2

struct UniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
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

    void createVertexBuffer();

    void createCommandBuffer();

    void createSyncObjects();

    // RENDER

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

    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
};

struct Vertex {
    glm::vec2 pos;

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 1> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 1> attributeDescriptions{};
        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R16G16_UINT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);
        /*attributeDescriptions[1].binding = 0; // resize the array, if you want to uncomment this
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, colour);*/
        return attributeDescriptions;
    }
};

const std::vector<Vertex> vertices = {
        {{50u,  50u}},
        {{50u,  0u}},
        {{100u,  50u}},
        {{0u, 50u}},
        {{50u, 100u}}
};

#endif //VIS_ANALYSES_H
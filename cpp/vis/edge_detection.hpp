#ifndef VIS_EDGE_DETECTION_H
#define VIS_EDGE_DETECTION_H

#include <android/native_window.h>
#include <atomic>
#include <media/NdkImage.h>

#include "../global_vk.hpp"

// debug the results using the analyses window
#define VIS_ED_ANALYSES true

const int WIDTH = 720; // size of rendered mandelbrot set.
const int HEIGHT = 720; // size of renderered mandelbrot set.
const int WORKGROUP_SIZE = 18; // Workgroup size in compute shader.

struct Pixel {
    float r, g, b, a;
};

class EdgeDetection {
public:
    EdgeDetection(AAssetManager *assets, ANativeWindow *analyses);

    void Process(AImage *image);

    ~EdgeDetection();


    std::atomic<bool> locked = false;
#if VIS_ED_ANALYSES
    ANativeWindow *analyses = nullptr;
#endif

private:
    void createInstance();

#if ENABLE_VALIDATION_LAYERS

    bool checkValidationLayerSupport();

    void setupDebugMessenger();

#endif

    void pickPhysicalDevice();

    void createLogicalDeviceAndQueue();

    uint32_t getComputeQueueFamilyIndex();

    void createBuffer();

    uint32_t findMemoryType(uint32_t memoryTypeBits);

    void createDescriptorSetLayout();

    void createDescriptorSet();

    void createComputePipeline(AAssetManager *newManager);

    void createCommandBuffer();

    void runCommandBuffer();


    VkInstance instance;

#if ENABLE_VALIDATION_LAYERS
    const std::vector<const char *> validationLayers = {"VK_LAYER_KHRONOS_validation"};
    VkDebugUtilsMessengerEXT debugMessenger;
#endif

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device;

    /** A queue supporting compute operations.
     * In order to execute commands on a device(GPU), the commands must be submitted
     * to a queue. The commands are stored in a command buffer, and this command buffer
     * is given to the queue.
     * There will be different kinds of queues on the device. Not all queues support
     * graphics operations, for instance. For this application, we at least want a queue
     * that supports compute operations. */
    VkQueue queue;
    uint32_t queueFamilyIndex;

    VkDescriptorPool descriptorPool;
    VkDescriptorSet descriptorSet;
    VkDescriptorSetLayout descriptorSetLayout;

    /** The pipeline specifies the pipeline that all graphics and compute commands pass though
     * in Vulkan. We will be creating a simple compute pipeline in this application. */
    VkPipeline pipeline;
    VkPipelineLayout pipelineLayout;

    /** The command buffer is used to record commands, that will be submitted to a queue.
     * To allocate such command buffers, we use a command pool. */
    VkCommandPool commandPool;
    VkCommandBuffer commandBuffer;

    /** The mandelbrot set will be rendered to this buffer.
     * The memory that backs the buffer is bufferMemory. */
    VkBuffer buffer;
    VkDeviceMemory bufferMemory;

    /** size of `buffer` in bytes */
    uint32_t bufferSize;

#if VIS_ED_ANALYSES
    ANativeWindow_Buffer analysesBuf{};
#endif
};

#endif //VIS_EDGE_DETECTION_H

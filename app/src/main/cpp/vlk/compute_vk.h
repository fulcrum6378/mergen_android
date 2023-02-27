#ifndef VLK_COMPUTE_VK_H
#define VLK_COMPUTE_VK_H

#include "global.h"

const int WIDTH = 3200; // Size of rendered mandelbrot set.
const int HEIGHT = 2400; // Size of renderered mandelbrot set.
const int WORKGROUP_SIZE = 18; // Workgroup size in compute shader.

struct Pixel {
    float r, g, b, a;
};

class ComputeVK {
public:
    void run(AAssetManager *assets);

private:
    void createInstance();

    void setupDebugMessenger();

    void pickPhysicalDevice();

    void createLogicalDeviceAndQueue();

    void createBuffer();

    void createDescriptorSetLayout();

    void createDescriptorSet();

    void createComputePipeline(AAssetManager *newManager);

    void createCommandBuffer();

    void runCommandBuffer();

    void saveRenderedImage();

    void cleanup();


    bool checkValidationLayerSupport();

    uint32_t getComputeQueueFamilyIndex();

    uint32_t findMemoryType(uint32_t memoryTypeBits, VkMemoryPropertyFlags properties);


    bool enableValidationLayers = true;
    const std::vector<const char *> validationLayers = {"VK_LAYER_KHRONOS_validation"};

    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;

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
};

#endif //VLK_COMPUTE_VK_H

#ifndef VIS_EDGE_DETECTION_H
#define VIS_EDGE_DETECTION_H

#include "../global_vk.hpp"
#include "global.hpp"

#define VIS_ED_N_BUFFERS 2u
/** W /= WORKGROUP_SIZE */
#define VIS_ED_WORKGROUP_COUNT 40u

class EdgeDetection {
public:
    EdgeDetection(AAssetManager *assets, uint32_t *img, uint32_t *edges);

    void runCommandBuffer();

    ~EdgeDetection();

private:
    void createInstance();

#if VK_VALIDATION_LAYERS

    bool checkValidationLayerSupport();

    void setupDebugMessenger();

#endif

    void pickPhysicalDevice();

    void createLogicalDeviceAndQueue();

    uint32_t getComputeQueueFamilyIndex();

    void createBuffer(VkBuffer &buffer, VkDeviceSize bufSize, VkDeviceMemory &bufMemory);

    uint32_t findMemoryType(uint32_t memoryTypeBits);

    void createDescriptorSetLayout();

    void createDescriptorPool();

    void createDescriptorSet();

    void createComputePipeline(AAssetManager *newManager);

    void createCommandBuffer();


    VkInstance instance;

#if VK_VALIDATION_LAYERS
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

    // The memory that backs the buffer is bufferMemory.
    VkDeviceMemory bufferInMemory;
    VkBuffer bufferIn;
    size_t bufferInSize;
    VkDeviceMemory bufferOutMemory;
    VkBuffer bufferOut;
    size_t bufferOutSize;

    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorPool descriptorPool;
    VkDescriptorSet descriptorSet;

    /** The pipeline specifies the pipeline that all graphics and compute commands pass though
     * in Vulkan. We will be creating a simple compute pipeline in this application. */
    VkPipeline pipeline;
    VkPipelineLayout pipelineLayout;

    /** The command buffer is used to record commands, that will be submitted to a queue.
     * To allocate such command buffers, we use a command pool. */
    VkCommandPool commandPool;
    VkCommandBuffer commandBuffer;

    // multidimensional array of pixels
    uint32_t *img_;
    // a map that highlights marginal pixels
    uint32_t *edges_;
    // virtual memory for transferring data into and out of GPU
    void *gpuData;
};

#endif //VIS_EDGE_DETECTION_H

#ifndef VIS_EDGE_DETECTION_H
#define VIS_EDGE_DETECTION_H

#include <android/native_window.h>
#include <atomic>
#include <media/NdkImage.h>

#include "../global_vk.hpp"
#include "global.hpp"

#define VIS_ED_N_BUFFERS 2u
#define VID_ED_WORKGROUP_SIZE 18.0f

struct Pixel {
    float r, g, b, a;
};

class EdgeDetection {
public:
    EdgeDetection(AAssetManager *assets, ANativeWindow *analyses);

    void Process(AImage *image);

    ~EdgeDetection();


    std::atomic<bool> locked = false;
#if VIS_ANALYSES
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

    void createBuffer(VkBuffer &buffer, VkDeviceSize bufSize, VkDeviceMemory &bufMemory);

    uint32_t findMemoryType(uint32_t memoryTypeBits);

    void createDescriptorSetLayout();

    void createDescriptorPool();

    void createDescriptorSet();

    void createComputePipeline(AAssetManager *newManager);

    void createCommandBuffer();


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

    // The memory that backs the buffer is bufferMemory.
    VkDeviceMemory bufferInMemory;
    VkBuffer bufferIn;
    uint32_t bufferInSize;
    VkDeviceMemory bufferOutMemory;
    VkBuffer bufferOut;
    uint32_t bufferOutSize;

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
    uint32_t arr[H][W]{};
    // maps pixels to their status of being border or not
    //uint8_t b_status[H][W]{};

#if VIS_ANALYSES
    ANativeWindow_Buffer analysesBuf{};
#endif
};

#endif //VIS_EDGE_DETECTION_H

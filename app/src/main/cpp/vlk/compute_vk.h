#ifndef VLK_COMPUTE_VK_H
#define VLK_COMPUTE_VK_H

#include "../global.h"
#include "global.h"

class ComputeVK {
public:
    void run();

private:
    void createInstance();

    void setupDebugMessenger();

    void pickPhysicalDevice();

    void createLogicalDeviceAndQueue();

    void createBuffer();

    void createDescriptorSetLayout();

    void createDescriptorSet();

    void createComputePipeline();

    void createCommandBuffer();

    void runCommandBuffer();

    void saveRenderedImage();

    void cleanup();


    static std::vector<const char *> getRequiredExtensions(bool _enableValidationLayers);

    bool checkValidationLayerSupport();

    uint32_t getComputeQueueFamilyIndex();


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

    /**
     * Groups of queues that have the same capabilities(for instance, they all supports
     * graphics and computer operations), are grouped into queue families.
     *
     * When submitting a command buffer, you must specify to which queue in the family
     * you are submitting to.
     * This variable keeps track of the index of that queue in its family. */
    uint32_t queueFamilyIndex;
};

#endif //VLK_COMPUTE_VK_H

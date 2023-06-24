#include "compute_vk.h"
#include "lodepng.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-pragmas"
#pragma ide diagnostic ignored "OCDFAInspection"
#pragma ide diagnostic ignored "ConstantConditionsOC"

// PUBLIC:

void ComputeVK::run(AAssetManager *assets) {
    // Buffer size of the storage buffer that will contain the rendered mandelbrot set.
    bufferSize = sizeof(Pixel) * WIDTH * HEIGHT;

    createInstance();
    setupDebugMessenger();
    pickPhysicalDevice();
    createLogicalDeviceAndQueue();
    createBuffer();
    createDescriptorSetLayout();
    createDescriptorSet();
    createComputePipeline(assets);
    createCommandBuffer();

    runCommandBuffer();
    saveRenderedImage();

    cleanup();
}

// PRIVATE (INIT):

void ComputeVK::createInstance() {
    assert(!enableValidationLayers || checkValidationLayerSupport());
    std::vector<const char *> requiredExtensions;
    if (enableValidationLayers) requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

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

    VkDeviceCreateInfo deviceCreateInfo = {};
    VkPhysicalDeviceFeatures deviceFeatures = {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    if (enableValidationLayers) { // for Vulkan versions compatibility (no longer needed)
        deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        deviceCreateInfo.ppEnabledLayerNames = validationLayers.data();
    } else deviceCreateInfo.enabledLayerCount = 0;
    deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

    VK_CHECK(vkCreateDevice(
            physicalDevice, &deviceCreateInfo, nullptr, &device));
    vkGetDeviceQueue(device, queueFamilyIndex, 0, &queue);
}

void ComputeVK::createBuffer() {
    VkBufferCreateInfo bufferCreateInfo = {};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = bufferSize; // buffer size in bytes.
    bufferCreateInfo.usage =
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT; // buffer is used as a storage buffer.
    bufferCreateInfo.sharingMode =
            VK_SHARING_MODE_EXCLUSIVE; // buffer is exclusive to a single queue family at a time.
    VK_CHECK(vkCreateBuffer(
            device, &bufferCreateInfo, nullptr, &buffer));

    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(device, buffer, &memoryRequirements);

    VkMemoryAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocateInfo.allocationSize = memoryRequirements.size; // specify required memory.
    allocateInfo.memoryTypeIndex = findMemoryType(
            memoryRequirements.memoryTypeBits,
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    VK_CHECK(vkAllocateMemory(device, &allocateInfo, nullptr, &bufferMemory));

    VK_CHECK(vkBindBufferMemory(device, buffer, bufferMemory, 0));
}

void ComputeVK::createDescriptorSetLayout() {
    VkDescriptorSetLayoutBinding descriptorSetLayoutBinding = {};
    descriptorSetLayoutBinding.binding = 0;
    descriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptorSetLayoutBinding.descriptorCount = 1;
    descriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {};
    descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetLayoutCreateInfo.bindingCount = 1; // only a single binding in this descriptor set layout.
    descriptorSetLayoutCreateInfo.pBindings = &descriptorSetLayoutBinding;
    VK_CHECK(vkCreateDescriptorSetLayout(
            device, &descriptorSetLayoutCreateInfo, nullptr,
            &descriptorSetLayout));
}

void ComputeVK::createDescriptorSet() {
    VkDescriptorPoolSize descriptorPoolSize = {};
    descriptorPoolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptorPoolSize.descriptorCount = 1;

    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
    descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolCreateInfo.maxSets = 1; // we only need to allocate one descriptor set from the pool.
    descriptorPoolCreateInfo.poolSizeCount = 1;
    descriptorPoolCreateInfo.pPoolSizes = &descriptorPoolSize;
    VK_CHECK(vkCreateDescriptorPool(
            device, &descriptorPoolCreateInfo, nullptr,
            &descriptorPool));

    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {};
    descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorSetAllocateInfo.descriptorPool = descriptorPool; // pool to allocate from.
    descriptorSetAllocateInfo.descriptorSetCount = 1; // allocate a single descriptor set.
    descriptorSetAllocateInfo.pSetLayouts = &descriptorSetLayout;
    VK_CHECK(vkAllocateDescriptorSets(
            device, &descriptorSetAllocateInfo, &descriptorSet));

    VkDescriptorBufferInfo descriptorBufferInfo = {};
    descriptorBufferInfo.buffer = buffer;
    descriptorBufferInfo.offset = 0;
    descriptorBufferInfo.range = bufferSize;

    VkWriteDescriptorSet writeDescriptorSet = {};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.dstSet = descriptorSet; // write to this descriptor set.
    writeDescriptorSet.dstBinding = 0; // write to the first, and only binding.
    writeDescriptorSet.descriptorCount = 1; // update a single descriptor.
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER; // storage buffer.
    writeDescriptorSet.pBufferInfo = &descriptorBufferInfo;
    vkUpdateDescriptorSets(
            device, 1, &writeDescriptorSet, 0,
            nullptr);
}

void ComputeVK::createComputePipeline(AAssetManager *assets) {
    auto code = LoadBinaryFileToVector(
            "shaders/mandelbrot.comp.spv", assets);
    VkShaderModule computeShaderModule;

    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());
    VK_CHECK(vkCreateShaderModule(
            device, &createInfo, nullptr, &computeShaderModule));

    VkPipelineShaderStageCreateInfo shaderStageCreateInfo = {};
    shaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStageCreateInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    shaderStageCreateInfo.module = computeShaderModule;
    shaderStageCreateInfo.pName = "main";

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.setLayoutCount = 1;
    pipelineLayoutCreateInfo.pSetLayouts = &descriptorSetLayout;
    VK_CHECK(vkCreatePipelineLayout(
            device, &pipelineLayoutCreateInfo, nullptr,
            &pipelineLayout));

    VkComputePipelineCreateInfo pipelineCreateInfo = {};
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineCreateInfo.stage = shaderStageCreateInfo;
    pipelineCreateInfo.layout = pipelineLayout;

    VK_CHECK(vkCreateComputePipelines(
            device, VK_NULL_HANDLE, 1, &pipelineCreateInfo,
            nullptr, &pipeline));
    vkDestroyShaderModule(device, computeShaderModule, nullptr);
}

void ComputeVK::createCommandBuffer() {
    VkCommandPoolCreateInfo commandPoolCreateInfo = {};
    commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolCreateInfo.flags = 0;
    commandPoolCreateInfo.queueFamilyIndex = queueFamilyIndex;
    VK_CHECK(vkCreateCommandPool(
            device, &commandPoolCreateInfo, nullptr, &commandPool));

    VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.commandPool = commandPool;
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocateInfo.commandBufferCount = 1;
    VK_CHECK(vkAllocateCommandBuffers(
            device, &commandBufferAllocateInfo, &commandBuffer));

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    // the buffer is only submitted and used once in this application.
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    VK_CHECK(vkBeginCommandBuffer(commandBuffer, &beginInfo));

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
    vkCmdBindDescriptorSets(
            commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout,
            0, 1, &descriptorSet, 0,
            nullptr);

    vkCmdDispatch(commandBuffer, (uint32_t) ceil(WIDTH / float(WORKGROUP_SIZE)),
                  (uint32_t) ceil(HEIGHT / float(WORKGROUP_SIZE)), 1);

    VK_CHECK(vkEndCommandBuffer(commandBuffer));
}

/** Finally submit the recorded command buffer to a queue. */
void ComputeVK::runCommandBuffer() {
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer; // the command buffer to submit.

    VkFence fence;
    VkFenceCreateInfo fenceCreateInfo = {};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = 0;
    VK_CHECK(vkCreateFence(device, &fenceCreateInfo, nullptr, &fence));

    // We submit the command buffer on the queue, at the same time giving a fence.
    VK_CHECK(vkQueueSubmit(queue, 1, &submitInfo, fence));

    /* The command will not have finished executing until the fence is signalled.
     * So we wait here.
     * We will directly after this read our buffer from the GPU,
     * and we will not be sure that the command has finished executing unless we wait for the fence.
     * Hence, we use a fence here. */
    VK_CHECK(vkWaitForFences(
            device, 1, &fence, VK_TRUE, 100000000000));

    vkDestroyFence(device, fence, nullptr);
}

void ComputeVK::saveRenderedImage() {
    void *mappedMemory = nullptr;
    // Map the buffer memory, so that we can read from it on the CPU.
    vkMapMemory(device, bufferMemory, 0, bufferSize, 0,
                &mappedMemory);
    auto *pmappedMemory = (Pixel *) mappedMemory;

    // Get the color data from the buffer, and cast it to bytes.
    // We save the data to a vector.
    std::vector<unsigned char> image;
    image.reserve(WIDTH * HEIGHT * 4);
    for (int i = 0; i < WIDTH * HEIGHT; i += 1) {
        image.push_back((unsigned char) (255.0f * (pmappedMemory[i].r)));
        image.push_back((unsigned char) (255.0f * (pmappedMemory[i].g)));
        image.push_back((unsigned char) (255.0f * (pmappedMemory[i].b)));
        image.push_back((unsigned char) (255.0f * (pmappedMemory[i].a)));
    }
    // Done reading, so unmap.
    vkUnmapMemory(device, bufferMemory);

    unsigned error = lodepng::encode(
            "/data/data/ir.mahdiparastesh.mergen/files/mandelbrot.png",
            image, WIDTH, HEIGHT);
    if (error) printf("encoder error %d: %s", error, lodepng_error_text(error));
}

void ComputeVK::cleanup() {
    vkFreeMemory(device, bufferMemory, nullptr);
    vkDestroyBuffer(device, buffer, nullptr);
    vkDestroyDescriptorPool(device, descriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    vkDestroyPipeline(device, pipeline, nullptr);
    vkDestroyCommandPool(device, commandPool, nullptr);
    vkDestroyDevice(device, nullptr);
    if (enableValidationLayers)
        DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    vkDestroyInstance(instance, nullptr);
}

// PRIVATE (MISC):

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
    ASSERT(i != queueFamilies.size(), "Could not find a queue family that supports operations")
    return i;
}

/** Find memory type with desired properties. */
uint32_t ComputeVK::findMemoryType(uint32_t memoryTypeBits, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i)
        if ((memoryTypeBits & (1 << i)) &&
            ((memoryProperties.memoryTypes[i].propertyFlags & properties) == properties))
            return i;
    return -1;
}

// STATIC:

#pragma clang diagnostic pop

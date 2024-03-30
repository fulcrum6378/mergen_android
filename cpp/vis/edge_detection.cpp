#include <cmath>
#include <cstring>

#include "edge_detection.hpp"

#pragma clang diagnostic push
#pragma ide diagnostic ignored "cppcoreguidelines-pro-type-member-init"

EdgeDetection::EdgeDetection(AAssetManager *assets, ANativeWindow *analyses) : analyses(analyses) {
#if VIS_ED_ANALYSES
    auto *out = static_cast<uint32_t *>(analysesBuf.bits);
    out += analysesBuf.width - 1;
    for (int32_t yy = 0; yy < analysesBuf.height; yy++) {
        for (int32_t xx = 0; xx < analysesBuf.width; xx++)
            out[xx * analysesBuf.stride] = 0x000000FF; // ABGR
        out -= 1; // move to the next column
    }
#endif

    createInstance();
#if ENABLE_VALIDATION_LAYERS
    setupDebugMessenger();
#endif
    pickPhysicalDevice();
    createLogicalDeviceAndQueue();
    bufferInSize = WIDTH * HEIGHT * 3u;
    createBuffer(bufferIn, bufferInSize, bufferInMemory);
    bufferOutSize = WIDTH * HEIGHT * sizeof(Pixel);
    createBuffer(bufferOut, bufferOutSize, bufferOutMemory);
    createDescriptorSetLayout();
    createDescriptorPool();
    createDescriptorSet();
    createComputePipeline(assets);
    createCommandBuffer();
}

#pragma clang diagnostic pop

void EdgeDetection::createInstance() {
    std::vector<const char *> requiredExtensions;
#if ENABLE_VALIDATION_LAYERS
    assert(checkValidationLayerSupport());
    requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &vkAppInfo;
    createInfo.enabledExtensionCount = (uint32_t) requiredExtensions.size();
    createInfo.ppEnabledExtensionNames = requiredExtensions.data();

#if ENABLE_VALIDATION_LAYERS
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
    createInfo.ppEnabledLayerNames = validationLayers.data();
    populateDebugMessengerCreateInfo(debugCreateInfo);
    createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *) &debugCreateInfo;
#else
    createInfo.enabledLayerCount = 0;
    createInfo.pNext = nullptr;
#endif
    VK_CHECK(vkCreateInstance(&createInfo, nullptr, &instance));
}

#if ENABLE_VALIDATION_LAYERS

bool EdgeDetection::checkValidationLayerSupport() {
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

void EdgeDetection::setupDebugMessenger() {
    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    populateDebugMessengerCreateInfo(createInfo);
    VK_CHECK(CreateDebugUtilsMessengerEXT(
            instance, &createInfo, nullptr, &debugMessenger));
}

#endif

void EdgeDetection::pickPhysicalDevice() {
    uint32_t deviceCount = 0u;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    assert(deviceCount > 0u);
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(
            instance, &deviceCount, devices.data());
    VkPhysicalDevice &first = devices[0];
    physicalDevice = first;
    assert(physicalDevice != VK_NULL_HANDLE);
}

void EdgeDetection::createLogicalDeviceAndQueue() {
    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueFamilyIndex = getComputeQueueFamilyIndex(); // find queue family with compute capability.
    queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
    queueCreateInfo.queueCount = 1u; // create one queue in this family. We don't need more.
    float queuePriorities = 1.0;  // we only have one queue, so this is not that imporant.
    queueCreateInfo.pQueuePriorities = &queuePriorities;

    VkDeviceCreateInfo deviceCreateInfo{};
    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
#if ENABLE_VALIDATION_LAYERS // for Vulkan versions compatibility (no longer needed)
    deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
    deviceCreateInfo.ppEnabledLayerNames = validationLayers.data();
#else
    deviceCreateInfo.enabledLayerCount = 0u;
#endif
    deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
    deviceCreateInfo.queueCreateInfoCount = 1u;
    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

    VK_CHECK(vkCreateDevice(
            physicalDevice, &deviceCreateInfo, nullptr, &device));
    vkGetDeviceQueue(device, queueFamilyIndex, 0u, &queue);
}

uint32_t EdgeDetection::getComputeQueueFamilyIndex() {
    uint32_t queueFamilyCount;
    vkGetPhysicalDeviceQueueFamilyProperties(
            physicalDevice, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(
            physicalDevice, &queueFamilyCount,
            queueFamilies.data());

    uint32_t i = 0u;
    for (; i < queueFamilies.size(); ++i) {
        VkQueueFamilyProperties props = queueFamilies[i];
        if (props.queueCount > 0u && (props.queueFlags & VK_QUEUE_COMPUTE_BIT))
            // found a queue with compute. We're done!
            break;
    }
    ASSERT(i != queueFamilies.size(), "Could not find a queue family that supports operations")
    return i;
}

void EdgeDetection::createBuffer(
        VkBuffer &buffer, VkDeviceSize bufSize, VkDeviceMemory &bufMemory) {
    VkBufferCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    createInfo.size = bufSize;
    createInfo.usage =
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT; // buffer is used as a storage buffer.
    createInfo.sharingMode =
            VK_SHARING_MODE_EXCLUSIVE; // buffer is exclusive to a single queue family at a time.
    VK_CHECK(vkCreateBuffer(
            device, &createInfo, nullptr, &buffer));

    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(device, buffer, &memoryRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memoryRequirements.size; // specify required memory.
    allocInfo.memoryTypeIndex = findMemoryType(
            memoryRequirements.memoryTypeBits);
    VK_CHECK(vkAllocateMemory(device, &allocInfo, nullptr, &bufMemory));

    VK_CHECK(vkBindBufferMemory(device, buffer, bufMemory, 0u));
}

/** Find memory type with desired properties. */
uint32_t EdgeDetection::findMemoryType(uint32_t memoryTypeBits) {
    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

    for (uint32_t i = 0u; i < memoryProperties.memoryTypeCount; ++i)
        if ((memoryTypeBits & (1 << i)) &&
            ((memoryProperties.memoryTypes[i].propertyFlags & 6) == 6))
            return i;
    return -1;
}

void EdgeDetection::createDescriptorSetLayout() {
    std::array<VkDescriptorSetLayoutBinding, VIS_ED_N_BUFFERS> layoutBindings{};
    for (size_t b = 0u; b < VIS_ED_N_BUFFERS; b++) {
        layoutBindings[b].binding = b;
        layoutBindings[b].descriptorCount = 1;
        layoutBindings[b].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        layoutBindings[b].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    }

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = layoutBindings.size();
    layoutInfo.pBindings = layoutBindings.data();
    VK_CHECK(vkCreateDescriptorSetLayout(
            device, &layoutInfo, nullptr, &descriptorSetLayout));
}

void EdgeDetection::createDescriptorPool() {
    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSize.descriptorCount = VIS_ED_N_BUFFERS;

    VkDescriptorPoolCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    createInfo.maxSets = VIS_ED_N_BUFFERS;
    createInfo.poolSizeCount = 1u;
    createInfo.pPoolSizes = &poolSize;
    VK_CHECK(vkCreateDescriptorPool(
            device, &createInfo, nullptr, &descriptorPool));
}

void EdgeDetection::createDescriptorSet() {
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool; // pool to allocate from.
    allocInfo.descriptorSetCount = 1u;
    allocInfo.pSetLayouts = &descriptorSetLayout;
    VK_CHECK(vkAllocateDescriptorSets(
            device, &allocInfo, &descriptorSet));

    std::array<VkDescriptorBufferInfo, VIS_ED_N_BUFFERS> bufferInfos{};
    bufferInfos[0].buffer = bufferIn;
    bufferInfos[0].offset = 0u;
    bufferInfos[0].range = bufferInSize;
    bufferInfos[1].buffer = bufferOut;
    bufferInfos[1].offset = 0u;
    bufferInfos[1].range = bufferOutSize;

    VkWriteDescriptorSet writeDescriptorSet{};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.dstSet = descriptorSet;
    writeDescriptorSet.dstBinding = 0u;
    writeDescriptorSet.descriptorCount = 1u;
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    writeDescriptorSet.pBufferInfo = bufferInfos.data();
    vkUpdateDescriptorSets(
            device, 1u, &writeDescriptorSet, 0u,
            nullptr);
}

void EdgeDetection::createComputePipeline(AAssetManager *assets) {
    auto code = LoadShaderCode(
            "shaders/edge_detection.comp.spv", assets);
    VkShaderModule computeShaderModule;

    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());
    VK_CHECK(vkCreateShaderModule(
            device, &createInfo, nullptr, &computeShaderModule));

    VkPipelineShaderStageCreateInfo shaderStageCreateInfo{};
    shaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStageCreateInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    shaderStageCreateInfo.module = computeShaderModule;
    shaderStageCreateInfo.pName = "main";

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.setLayoutCount = 1u;
    pipelineLayoutCreateInfo.pSetLayouts = &descriptorSetLayout;
    VK_CHECK(vkCreatePipelineLayout(
            device, &pipelineLayoutCreateInfo, nullptr,
            &pipelineLayout));

    VkComputePipelineCreateInfo pipelineCreateInfo{};
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineCreateInfo.stage = shaderStageCreateInfo;
    pipelineCreateInfo.layout = pipelineLayout;

    VK_CHECK(vkCreateComputePipelines(
            device, VK_NULL_HANDLE, 1u, &pipelineCreateInfo,
            nullptr, &pipeline));
    vkDestroyShaderModule(device, computeShaderModule, nullptr);
}

void EdgeDetection::createCommandBuffer() {
    VkCommandPoolCreateInfo commandPoolCreateInfo{};
    commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolCreateInfo.flags = 0;
    commandPoolCreateInfo.queueFamilyIndex = queueFamilyIndex;
    VK_CHECK(vkCreateCommandPool(
            device, &commandPoolCreateInfo, nullptr, &commandPool));

    VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.commandPool = commandPool;
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocateInfo.commandBufferCount = 1u;
    VK_CHECK(vkAllocateCommandBuffers(
            device, &commandBufferAllocateInfo, &commandBuffer));

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    // the buffer is only submitted and used once in this application.
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    VK_CHECK(vkBeginCommandBuffer(commandBuffer, &beginInfo));

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
    vkCmdBindDescriptorSets(
            commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout,
            0u, 1u, &descriptorSet,
            0u, nullptr);

    vkCmdDispatch(commandBuffer, (uint32_t) std::ceil(WIDTH / float(WORKGROUP_SIZE)),
                  (uint32_t) std::ceil(HEIGHT / float(WORKGROUP_SIZE)), 1);

    VK_CHECK(vkEndCommandBuffer(commandBuffer));
}


void EdgeDetection::Process(AImage *image) {
    locked = true;

    /*void *data;
    vkMapMemory(device, bufferInMemory, 0, bufferInSize, 0, &data);
    memcpy(data, vertices.data(), bufferInSize);
    vkUnmapMemory(device, bufferInMemory);*/


    /**** BEGIN RUN COMMAND BUFFER ****/

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1u;
    submitInfo.pCommandBuffers = &commandBuffer; // the command buffer to submit.

    VkFence fence;
    VkFenceCreateInfo fenceCreateInfo{};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = 0;
    VK_CHECK(vkCreateFence(device, &fenceCreateInfo, nullptr, &fence));

    // We submit the command buffer on the queue, at the same time giving a fence.
    VK_CHECK(vkQueueSubmit(queue, 1u, &submitInfo, fence));

    /* The command will not have finished executing until the fence is signalled.
     * So we wait here.
     * We will directly after this read our buffer from the GPU,
     * and we will not be sure that the command has finished executing unless we wait for the fence.
     * Hence, we use a fence here. */
    VK_CHECK(vkWaitForFences(
            device, 1, &fence, VK_TRUE, 100000000000));

    vkDestroyFence(device, fence, nullptr);

    /**** END RUN COMMAND BUFFER ****/


    void *mappedMemory = nullptr;
    // Map the buffer memory, so that we can read from it on the CPU.
    vkMapMemory(device, bufferOutMemory, 0u, bufferOutSize, 0,
                &mappedMemory);
    auto *pmappedMemory = (Pixel *) mappedMemory;

    // Get the color data from the buffer, and cast it to bytes.
    // We save the data to a vector.
    std::vector<unsigned char> img;
    img.reserve(WIDTH * HEIGHT * 4u);
    for (int i = 0; i < WIDTH * HEIGHT; i += 1) {
        img.push_back((unsigned char) (255.0f * (pmappedMemory[i].r)));
        img.push_back((unsigned char) (255.0f * (pmappedMemory[i].g)));
        img.push_back((unsigned char) (255.0f * (pmappedMemory[i].b)));
        img.push_back((unsigned char) (255.0f * (pmappedMemory[i].a)));
    }
    // Done reading, so unmap.
    vkUnmapMemory(device, bufferOutMemory);

#if VIS_ED_ANALYSES
    ANativeWindow_acquire(analyses);
    if (ANativeWindow_lock(analyses, &analysesBuf, nullptr) == 0) {
        auto *out = static_cast<uint8_t *>(analysesBuf.bits);
        out += (analysesBuf.width * 4) - 4;
        int index;
        auto it = img.begin();
        for (int32_t yy = 0; yy < analysesBuf.height; yy++) {
            for (int32_t xx = 0; xx < analysesBuf.width; xx++) {
                index = xx * analysesBuf.stride * 4;
                out[index + 3] = *it;
                it++;
                out[index + 2] = *it;
                it++;
                out[index + 1] = *it;
                it++;
                out[index] = *it;
                it++;
            }
            out -= 4; // move to the next column
        }
        ANativeWindow_unlockAndPost(analyses);
    }
    ANativeWindow_release(analyses);
#endif
    AImage_delete(image);
    //locked = false;
}

EdgeDetection::~EdgeDetection() {
    vkFreeMemory(device, bufferOutMemory, nullptr);
    vkDestroyBuffer(device, bufferOut, nullptr);
    vkDestroyBuffer(device, bufferIn, nullptr);
    vkDestroyDescriptorPool(device, descriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    vkDestroyPipeline(device, pipeline, nullptr);
    vkDestroyCommandPool(device, commandPool, nullptr);
    vkDestroyDevice(device, nullptr);
#if ENABLE_VALIDATION_LAYERS
    DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
#endif
    vkDestroyInstance(instance, nullptr);
}

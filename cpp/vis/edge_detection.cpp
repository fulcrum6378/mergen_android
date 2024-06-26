#include <array>
#include <cmath>
#include <cstring>

#include "edge_detection.hpp"

using namespace std;

#pragma clang diagnostic push
#pragma ide diagnostic ignored "cppcoreguidelines-pro-type-member-init"

EdgeDetection::EdgeDetection(AAssetManager *assets, uint32_t *img, uint32_t *edges) : img_(img), edges_(edges) {

    createInstance();
#if VK_VALIDATION_LAYERS
    setupDebugMessenger();
#endif
    pickPhysicalDevice();
    createLogicalDeviceAndQueue();
    bufferInSize = H * W * 4;
    createBuffer(bufferIn, bufferInSize, bufferInMemory);
    bufferOutSize = H * W * 4;
    createBuffer(bufferOut, bufferOutSize, bufferOutMemory);
    createDescriptorSetLayout();
    createDescriptorPool();
    createDescriptorSet();
    createComputePipeline(assets);
    createCommandBuffer();
}

#pragma clang diagnostic pop

void EdgeDetection::createInstance() {
    vector<const char *> requiredExtensions;
#if VK_VALIDATION_LAYERS
    assert(checkValidationLayerSupport());
    requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &vkAppInfo;
    createInfo.enabledExtensionCount = (uint32_t) requiredExtensions.size();
    createInfo.ppEnabledExtensionNames = requiredExtensions.data();

#if VK_VALIDATION_LAYERS
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

#if VK_VALIDATION_LAYERS

bool EdgeDetection::checkValidationLayerSupport() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    vector<VkLayerProperties> availableLayers(layerCount);
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
    vector<VkPhysicalDevice> devices(deviceCount);
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
#if VK_VALIDATION_LAYERS // for Vulkan versions compatibility (no longer needed)
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
    vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
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
            ((memoryProperties.memoryTypes[i].propertyFlags & 6u) == 6u))
            return i;
    return -1;
}

void EdgeDetection::createDescriptorSetLayout() {
    array<VkDescriptorSetLayoutBinding, VIS_ED_N_BUFFERS> layoutBindings{};
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

    array<VkDescriptorBufferInfo, VIS_ED_N_BUFFERS> bufferInfos{};
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
    writeDescriptorSet.descriptorCount = 2u;
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    writeDescriptorSet.pBufferInfo = bufferInfos.data();
    vkUpdateDescriptorSets(
            device, 1u, &writeDescriptorSet, 0u,
            nullptr);
}

void EdgeDetection::createComputePipeline(AAssetManager *assets) { // edge_detection.spv
    auto code = LoadShaderCode("shaders/edge_detection.comp.spv", assets);
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
    commandPoolCreateInfo.flags = 0u;
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
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    VK_CHECK(vkBeginCommandBuffer(commandBuffer, &beginInfo));

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
    vkCmdBindDescriptorSets(
            commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout,
            0u, 1u, &descriptorSet,
            0u, nullptr);

    vkCmdDispatch(commandBuffer,
                  VIS_ED_WORKGROUP_COUNT, VIS_ED_WORKGROUP_COUNT, 1u);

    VK_CHECK(vkEndCommandBuffer(commandBuffer));
}


void EdgeDetection::runCommandBuffer() {

    // write image data `img` to the GPU memory
    vkMapMemory(device, bufferInMemory, 0u, bufferInSize, 0u, &gpuData);
    memcpy(gpuData, img_, bufferInSize);
    vkUnmapMemory(device, bufferInMemory);

    // prepare for submission
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1u;
    submitInfo.pCommandBuffers = &commandBuffer;

    VkFence fence;
    VkFenceCreateInfo fenceCreateInfo{};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = 0u;
    VK_CHECK(vkCreateFence(device, &fenceCreateInfo, nullptr, &fence));

    // submit the work and wait for it
    VK_CHECK(vkQueueSubmit(queue, 1u, &submitInfo, fence));
    //LOGI("Fence submitted.");
    VK_CHECK(vkWaitForFences(
            device, 1u, &fence, VK_TRUE, 1000000000u)); // 1 second
    //LOGI("Fence done.");
    vkDestroyFence(device, fence, nullptr);

    // read processed data `edges` from the GPU memory
    vkMapMemory(device, bufferOutMemory, 0u, bufferOutSize, 0u, &gpuData);
    memcpy(edges_, gpuData, bufferOutSize);
    vkUnmapMemory(device, bufferOutMemory);
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
#if VK_VALIDATION_LAYERS
    DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
#endif
    vkDestroyInstance(instance, nullptr);
}

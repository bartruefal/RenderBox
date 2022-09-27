#include "vk_helpers.h"

uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t memoryTypeMask, VkMemoryPropertyFlags memoryFlags){
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for (int i{}; i < memProperties.memoryTypeCount; i++){
        if ((memoryTypeMask & (1 << i)) &&
            (memProperties.memoryTypes[i].propertyFlags & memoryFlags)){
            return i;
        }
    }

    assert(!"Memory type not found!");
    return -1;
}

VkDeviceMemory allocateSimpleUBOs(VulkanState vkState, SimpleUBO* UBOs, uint32_t numUBOs, VkDeviceSize UBOsize, VkMemoryPropertyFlags memoryFlags){
    for (int i{}; i < numUBOs; i++)
    {
        VkBufferCreateInfo bufferInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
        bufferInfo.size = UBOsize;
        bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        bufferInfo.queueFamilyIndexCount = 1;
        bufferInfo.pQueueFamilyIndices = &vkState.renderQueueFamilyID;

        VK_CHECK(vkCreateBuffer(vkState.device, &bufferInfo, nullptr, &UBOs[i].buffer));
    }

    VkDeviceMemory UBOMemory{};

    VkMemoryRequirements bufferReqs{};
    vkGetBufferMemoryRequirements(vkState.device, UBOs[0].buffer, &bufferReqs);

    VkMemoryAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
    allocInfo.memoryTypeIndex = findMemoryType(vkState.physicalDevice,
                                                bufferReqs.memoryTypeBits,
                                                memoryFlags);
    allocInfo.allocationSize = bufferReqs.size * numUBOs;

    VK_CHECK(vkAllocateMemory(vkState.device, &allocInfo, nullptr, &UBOMemory));

    void* mappedData{};
    VK_CHECK(vkMapMemory(vkState.device, UBOMemory, 0, bufferReqs.size * numUBOs, 0, &mappedData));

    for (int i{}; i < numUBOs; i++)
    {
        VkDeviceSize offset{i * bufferReqs.size};
        UBOs[i].data = (uint8_t*)mappedData + i * bufferReqs.size;

        VK_CHECK(vkBindBufferMemory(vkState.device, UBOs[i].buffer, UBOMemory, offset));
    }

    return UBOMemory;
}

void destroySimpleUBOs(VkDevice device, SimpleUBO* UBOs, uint32_t numBuffers, VkDeviceMemory UBOmemory){
    vkUnmapMemory(device, UBOmemory);

    for (int i{}; i < numBuffers; i++){
        vkDestroyBuffer(device, UBOs[i].buffer, nullptr);
    }
}
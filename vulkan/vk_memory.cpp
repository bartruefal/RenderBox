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

Buffer createBuffer(VulkanState vkState, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memoryFlags){
    Buffer buffer{};

    VkBufferCreateInfo bufferInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.queueFamilyIndexCount = 1;
    bufferInfo.pQueueFamilyIndices = &vkState.renderQueueFamilyID;

    VK_CHECK(vkCreateBuffer(vkState.device, &bufferInfo, nullptr, &buffer.buffer));

    VkMemoryRequirements bufferReqs{};
    vkGetBufferMemoryRequirements(vkState.device, buffer.buffer, &bufferReqs);

    VkMemoryAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
    allocInfo.memoryTypeIndex = findMemoryType(vkState.physicalDevice,
                                                bufferReqs.memoryTypeBits,
                                                memoryFlags);
    allocInfo.allocationSize = bufferReqs.size;

    VK_CHECK(vkAllocateMemory(vkState.device, &allocInfo, nullptr, &buffer.memory));
    VK_CHECK(vkBindBufferMemory(vkState.device, buffer.buffer, buffer.memory, 0));
    VK_CHECK(vkMapMemory(vkState.device, buffer.memory, 0, bufferReqs.size, 0, &buffer.data));

    return buffer;
}

void destroyBuffer(VkDevice device, Buffer buffer){
    vkFreeMemory(device, buffer.memory, nullptr);
    vkDestroyBuffer(device, buffer.buffer, nullptr);
}
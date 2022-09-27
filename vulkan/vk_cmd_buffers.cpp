#include "vk_helpers.h"

VkCommandPool allocateCommandBuffers(VulkanState vkState, VkCommandBuffer* cmdBuffers, uint32_t numCmdBuffers){
    VkCommandPoolCreateInfo cmdPoolCreateInfo{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
    cmdPoolCreateInfo.queueFamilyIndex = vkState.renderQueueFamilyID;
    cmdPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    VkCommandPool cmdPool{};
    VK_CHECK(vkCreateCommandPool(vkState.device, &cmdPoolCreateInfo, nullptr, &cmdPool));

    VkCommandBufferAllocateInfo cmdBuffersAllocInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
    cmdBuffersAllocInfo.commandBufferCount = numCmdBuffers;
    cmdBuffersAllocInfo.commandPool = cmdPool;
    cmdBuffersAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    VK_CHECK(vkAllocateCommandBuffers(vkState.device, &cmdBuffersAllocInfo, cmdBuffers));

    return cmdPool;
}
#include "vk_helpers.h"

VkQueryPool createTimestampQueryPool(VkDevice device, uint32_t queryCount){
    VkQueryPoolCreateInfo createInfo{ VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO };
    createInfo.queryType = VK_QUERY_TYPE_TIMESTAMP;
    createInfo.queryCount = queryCount;

    VkQueryPool queryPool{};
    VK_CHECK(vkCreateQueryPool(device, &createInfo, nullptr, &queryPool));

    return queryPool;
}

void destroyQueryPool(VkDevice device, VkQueryPool queryPool){
    vkDestroyQueryPool(device, queryPool, nullptr);
}
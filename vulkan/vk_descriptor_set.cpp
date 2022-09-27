#include "vk_helpers.h"

VkDescriptorPool allocateUBODescriptorSets(VkDevice device, VkDescriptorSetLayout descrSetLayout,
                                           VkDescriptorSet* descrSets, uint32_t numDescrSets){
    VkDescriptorPoolSize descrPoolSize{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, numDescrSets };

    VkDescriptorPoolCreateInfo descrPoolCreateInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
    descrPoolCreateInfo.maxSets = numDescrSets;
    descrPoolCreateInfo.poolSizeCount = 1;
    descrPoolCreateInfo.pPoolSizes = &descrPoolSize;

    VkDescriptorPool descrPool{};
    VK_CHECK(vkCreateDescriptorPool(device, &descrPoolCreateInfo, nullptr, &descrPool));

    std::vector<VkDescriptorSetLayout> triangleDescrSetLayouts(numDescrSets, descrSetLayout);

    VkDescriptorSetAllocateInfo descrSetAllocInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
    descrSetAllocInfo.descriptorPool = descrPool;
    descrSetAllocInfo.descriptorSetCount = numDescrSets;
    descrSetAllocInfo.pSetLayouts = triangleDescrSetLayouts.data();

    VK_CHECK(vkAllocateDescriptorSets(device, &descrSetAllocInfo, descrSets));

    return descrPool;
}

void updateUBODescriptorSets(VkDevice device, VkDescriptorSet* descrSets, SimpleUBO* UBOs, uint32_t numUBOs, VkDeviceSize UBOsize){
    std::vector<VkDescriptorBufferInfo> bufferInfos(numUBOs);
    for (int i{}; i < numUBOs; i++)
    {
        bufferInfos[i].buffer = UBOs[i].buffer;
        bufferInfos[i].offset = 0;
        bufferInfos[i].range = UBOsize;
    }

    std::vector<VkWriteDescriptorSet> descrWrites(numUBOs);
    for (int i{}; i < numUBOs; i++)
    {
        descrWrites[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descrWrites[i].dstSet = descrSets[i];
        descrWrites[i].dstBinding = 0;
        descrWrites[i].dstArrayElement = 0;
        descrWrites[i].descriptorCount = 1;
        descrWrites[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descrWrites[i].pBufferInfo = &bufferInfos[i];
    }

    vkUpdateDescriptorSets(device, numUBOs, descrWrites.data(), 0, nullptr);
}
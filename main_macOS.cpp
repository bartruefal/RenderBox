#include "vulkan/vk_helpers.h"
#include "vulkan/vk_init.cpp"
#include "vulkan/vk_swapchain.cpp"
#include "vulkan/vk_pipeline.cpp"
#include "vulkan/vk_renderpass.cpp"
#include "vulkan/vk_descriptor_set.cpp"
#include "vulkan/vk_cmd_buffers.cpp"
#include "vulkan/vk_memory.cpp"
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <vector>

struct TriangleUniformData{
    float time;
};

int main() {
    int glfwInitResult{ glfwInit() };
    assert(glfwInitResult == GLFW_TRUE);

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(1024, 768, "Render Box", nullptr, nullptr);

    VulkanState vkState{ initializeVulkanState() };
    VulkanSwapchain vkSwapchain{ createSwapchain(window, vkState) };

    std::vector<VkCommandBuffer> cmdBuffers(vkSwapchain.images.size());
    VkCommandPool cmdPool{ allocateCommandBuffers(vkState, cmdBuffers.data(), cmdBuffers.size()) };

    VkSemaphoreCreateInfo semCreateInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
    VkSemaphore imageAcquireSemaphore{};
    VK_CHECK(vkCreateSemaphore(vkState.device, &semCreateInfo, nullptr, &imageAcquireSemaphore));

    VkSemaphore imageReleaseSemaphore{};
    VK_CHECK(vkCreateSemaphore(vkState.device, &semCreateInfo, nullptr, &imageReleaseSemaphore));

    VkFenceCreateInfo fenceCreateInfo{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    std::vector<VkFence> fences(vkSwapchain.images.size());
    for (int i{}; i < vkSwapchain.images.size(); i++){
        VK_CHECK(vkCreateFence(vkState.device, &fenceCreateInfo, nullptr, &fences[i]));
    }

    VkRenderPass triangleRenderPass{ createRenderPass(vkState.device, vkSwapchain.surfaceFormat.format) };

    std::vector<VkFramebuffer> framebuffers(vkSwapchain.images.size());
    for (int i{}; i < vkSwapchain.images.size(); i++){
        framebuffers[i] = createFramebuffer(vkState.device, triangleRenderPass, vkSwapchain.imageViews[i],
                                            vkSwapchain.surfaceFormat.format, vkSwapchain.extent);
    }

    std::vector<SimpleUBO> triangleUBOs(vkSwapchain.images.size());
    VkDeviceMemory triangleMemory{ allocateSimpleUBOs(vkState, triangleUBOs.data(),
                                                      vkSwapchain.images.size(), sizeof(TriangleUniformData),
                                                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                                      VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) };

    VkDescriptorSetLayout triangleDescrLayout{};
    {
        VkDescriptorSetLayoutBinding binding{};
        binding.binding = 0;
        binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        binding.descriptorCount = 1;
        binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        VkDescriptorSetLayoutCreateInfo descrSetLayoutInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
        descrSetLayoutInfo.bindingCount = 1;
        descrSetLayoutInfo.pBindings = &binding;

        VK_CHECK(vkCreateDescriptorSetLayout(vkState.device, &descrSetLayoutInfo, nullptr, &triangleDescrLayout));
    }

    std::vector<VkDescriptorSet> triangleDescrSets(vkSwapchain.images.size());
    VkDescriptorPool triangleDescrPool{ allocateUBODescriptorSets(vkState.device, triangleDescrLayout, 
                                                               triangleDescrSets.data(), triangleDescrSets.size()) };

    updateUBODescriptorSets(vkState.device, triangleDescrSets.data(), triangleUBOs.data(),
                            triangleUBOs.size(), sizeof(TriangleUniformData));

    VkPipelineLayout trianglePipelineLayout{};
    {
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &triangleDescrLayout;

        VK_CHECK(vkCreatePipelineLayout(vkState.device, &pipelineLayoutInfo, nullptr, &trianglePipelineLayout));
    }

    VkViewport viewport{ 0.0f, 0.0f, (float)vkSwapchain.extent.width, (float)vkSwapchain.extent.height, 0.0f, 1.0f };
    GraphicsPipeline trianglePipeline{ createGraphicsPipeline(vkState.device, triangleRenderPass, viewport, trianglePipelineLayout) };

    // Main Loop
    while (!glfwWindowShouldClose(window)){
        uint32_t nextImageID{};
        VkResult acquireRes{vkAcquireNextImageKHR(vkState.device, vkSwapchain.swapchain, -1, imageAcquireSemaphore, VK_NULL_HANDLE, &nextImageID)};
        assert(acquireRes == VK_SUCCESS || acquireRes == VK_SUBOPTIMAL_KHR);

        VK_CHECK(vkWaitForFences(vkState.device, 1, &fences[nextImageID], VK_FALSE, -1));
        VK_CHECK(vkResetFences(vkState.device, 1, &fences[nextImageID]));

        ((TriangleUniformData*)(triangleUBOs[nextImageID].data))->time += 0.02f;

        VkCommandBufferBeginInfo cmdBeginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
        cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        VK_CHECK(vkBeginCommandBuffer(cmdBuffers[nextImageID], &cmdBeginInfo));
        {
            {
                VkImageMemoryBarrier presentToRenderBarrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
                presentToRenderBarrier.srcAccessMask = 0;
                presentToRenderBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                presentToRenderBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                presentToRenderBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                presentToRenderBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                presentToRenderBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                presentToRenderBarrier.image = vkSwapchain.images[nextImageID];
                presentToRenderBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                presentToRenderBarrier.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
                presentToRenderBarrier.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;

                vkCmdPipelineBarrier(cmdBuffers[nextImageID],
                                    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                    VK_DEPENDENCY_BY_REGION_BIT,
                                    0, nullptr, 0, nullptr, 1, &presentToRenderBarrier);
            }

            VkClearValue clearValue{{{0.1f, 0.1f, 0.1f, 1.0f}}};

            VkRenderPassBeginInfo renderPassBeginInfo{ VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
            renderPassBeginInfo.renderPass = triangleRenderPass;
            renderPassBeginInfo.framebuffer = framebuffers[nextImageID];
            renderPassBeginInfo.renderArea.extent.width = vkSwapchain.extent.width;
            renderPassBeginInfo.renderArea.extent.height = vkSwapchain.extent.height;
            renderPassBeginInfo.clearValueCount = 1;
            renderPassBeginInfo.pClearValues = &clearValue;

            vkCmdBeginRenderPass(cmdBuffers[nextImageID], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

            vkCmdBindDescriptorSets(cmdBuffers[nextImageID], VK_PIPELINE_BIND_POINT_GRAPHICS, trianglePipelineLayout, 0, 1, &triangleDescrSets[nextImageID], 0, nullptr);

            vkCmdBindPipeline(cmdBuffers[nextImageID], VK_PIPELINE_BIND_POINT_GRAPHICS, trianglePipeline.pipeline);
            vkCmdDraw(cmdBuffers[nextImageID], 3, 1, 0, 0);

            vkCmdEndRenderPass(cmdBuffers[nextImageID]);

            {
                VkImageMemoryBarrier renderToPresentBarrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
                renderToPresentBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                renderToPresentBarrier.dstAccessMask = 0;
                renderToPresentBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                renderToPresentBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
                renderToPresentBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                renderToPresentBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                renderToPresentBarrier.image = vkSwapchain.images[nextImageID];
                renderToPresentBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                renderToPresentBarrier.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
                renderToPresentBarrier.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;

                vkCmdPipelineBarrier(cmdBuffers[nextImageID],
                                    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                    VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                                    VK_DEPENDENCY_BY_REGION_BIT,
                                    0, nullptr, 0, nullptr, 1, &renderToPresentBarrier);
            }
        }
        VK_CHECK(vkEndCommandBuffer(cmdBuffers[nextImageID]));

        VkPipelineStageFlags waitStages[]{
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
        };

        VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = &imageAcquireSemaphore;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &cmdBuffers[nextImageID];
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &imageReleaseSemaphore;

        VK_CHECK(vkQueueSubmit(vkState.renderQueue, 1, &submitInfo, fences[nextImageID]));

        VkPresentInfoKHR presentInfo{ VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &imageReleaseSemaphore;
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &vkSwapchain.swapchain;
        presentInfo.pImageIndices = &nextImageID;

        VkResult presentRes{ vkQueuePresentKHR(vkState.renderQueue, &presentInfo) };
        assert(presentRes == VK_SUCCESS || presentRes == VK_SUBOPTIMAL_KHR);

        glfwPollEvents();
    }
 
    {
        VK_CHECK(vkDeviceWaitIdle(vkState.device));

        destroyPipeline(vkState.device, trianglePipeline);
        vkDestroyPipelineLayout(vkState.device, trianglePipelineLayout, nullptr);

        destroySimpleUBOs(vkState.device, triangleUBOs.data(), triangleUBOs.size(), triangleMemory);

        vkDestroyDescriptorPool(vkState.device, triangleDescrPool, nullptr);

        vkDestroyDescriptorSetLayout(vkState.device, triangleDescrLayout, nullptr);

        for (int i{}; i < vkSwapchain.images.size(); i++){
            vkDestroyFramebuffer(vkState.device, framebuffers[i], nullptr);
        }

        vkDestroyRenderPass(vkState.device, triangleRenderPass, nullptr);

        for (int i{}; i < vkSwapchain.images.size(); i++){
            vkDestroyFence(vkState.device, fences[i], nullptr);
        }

        vkDestroySemaphore(vkState.device, imageReleaseSemaphore, nullptr);
        vkDestroySemaphore(vkState.device, imageAcquireSemaphore, nullptr);

        vkDestroyCommandPool(vkState.device, cmdPool, nullptr);

        destroySwapchain(vkState.instance, vkState.device, vkSwapchain);
        destroyVulkanState(vkState);
    }

    glfwDestroyWindow(window);

    glfwTerminate();

    return 0;
}
#include "vulkan/vk_helpers.h"
#include "vulkan/vk_init.cpp"
#include <GLFW/glfw3.h>
#include <vector>

int main() {
    int glfwInitResult{glfwInit()};
    assert(glfwInitResult == GLFW_TRUE);

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(1024, 768, "Vulkan Test", nullptr, nullptr);

    VulkanState vkState{ initializeVulkanState(window) };

    uint32_t swapchainImageCount{};
    VK_CHECK(vkGetSwapchainImagesKHR(vkState.device, vkState.swapchain, &swapchainImageCount, nullptr));

    std::vector<VkImage> swapchainImages(swapchainImageCount);
    VK_CHECK(vkGetSwapchainImagesKHR(vkState.device, vkState.swapchain, &swapchainImageCount, swapchainImages.data()));

    VkCommandPoolCreateInfo cmdPoolCreateInfo{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
    cmdPoolCreateInfo.queueFamilyIndex = vkState.renderQueueFamilyID;
    cmdPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    VkCommandPool cmdPool{VK_NULL_HANDLE};
    VK_CHECK(vkCreateCommandPool(vkState.device, &cmdPoolCreateInfo, nullptr, &cmdPool));

    VkCommandBufferAllocateInfo cmdBuffersAllocInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
    cmdBuffersAllocInfo.commandBufferCount = swapchainImageCount;
    cmdBuffersAllocInfo.commandPool = cmdPool;
    cmdBuffersAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    std::vector<VkCommandBuffer> cmdBuffers(swapchainImageCount);
    VK_CHECK(vkAllocateCommandBuffers(vkState.device, &cmdBuffersAllocInfo, cmdBuffers.data()));

    VkSemaphoreCreateInfo semCreateInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
    VkSemaphore imageAcquireSemaphore{ VK_NULL_HANDLE };
    VK_CHECK(vkCreateSemaphore(vkState.device, &semCreateInfo, nullptr, &imageAcquireSemaphore));

    VkSemaphore imageReleaseSemaphore{ VK_NULL_HANDLE };
    VK_CHECK(vkCreateSemaphore(vkState.device, &semCreateInfo, nullptr, &imageReleaseSemaphore));

    VkFenceCreateInfo fenceCreateInfo{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    std::vector<VkFence> fences(swapchainImageCount);
    for (int i{}; i < swapchainImageCount; i++){
        VK_CHECK(vkCreateFence(vkState.device, &fenceCreateInfo, nullptr, &fences[i]));
    }

    while(!glfwWindowShouldClose(window)){
        uint32_t nextImageID{};
        VkResult acquireRes{vkAcquireNextImageKHR(vkState.device, vkState.swapchain, -1, imageAcquireSemaphore, VK_NULL_HANDLE, &nextImageID)};
        assert(acquireRes == VK_SUCCESS || acquireRes == VK_SUBOPTIMAL_KHR);

        VK_CHECK(vkWaitForFences(vkState.device, 1, &fences[nextImageID], VK_FALSE, -1));
        VK_CHECK(vkResetFences(vkState.device, 1, &fences[nextImageID]));

        VkCommandBufferBeginInfo cmdBeginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
        cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        VK_CHECK(vkBeginCommandBuffer(cmdBuffers[nextImageID], &cmdBeginInfo));
        {
            VkImageSubresourceRange imageRange{ VK_IMAGE_ASPECT_COLOR_BIT };
            imageRange.levelCount = 1;
            imageRange.layerCount = 1;

            VkImageMemoryBarrier imageMemoryBarrier1{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
            imageMemoryBarrier1.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
            imageMemoryBarrier1.dstAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
            imageMemoryBarrier1.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imageMemoryBarrier1.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            imageMemoryBarrier1.srcQueueFamilyIndex = vkState.renderQueueFamilyID;
            imageMemoryBarrier1.dstQueueFamilyIndex = vkState.renderQueueFamilyID;
            imageMemoryBarrier1.image = swapchainImages[nextImageID];
            imageMemoryBarrier1.subresourceRange = imageRange;

            vkCmdPipelineBarrier(cmdBuffers[nextImageID],
                                 VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                 VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                 VK_DEPENDENCY_BY_REGION_BIT,
                                 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier1);

            VkClearColorValue clearColor{{0.2, 1, 0.4, 1}};
            vkCmdClearColorImage(cmdBuffers[nextImageID], swapchainImages[nextImageID], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearColor, 1, &imageRange);

            VkImageMemoryBarrier imageMemoryBarrier2{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
            imageMemoryBarrier2.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
            imageMemoryBarrier2.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
            imageMemoryBarrier2.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            imageMemoryBarrier2.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            imageMemoryBarrier2.srcQueueFamilyIndex = vkState.renderQueueFamilyID;
            imageMemoryBarrier2.dstQueueFamilyIndex = vkState.renderQueueFamilyID;
            imageMemoryBarrier2.image = swapchainImages[nextImageID];
            imageMemoryBarrier2.subresourceRange = imageRange;

            vkCmdPipelineBarrier(cmdBuffers[nextImageID],
                                 VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                 VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                 VK_DEPENDENCY_BY_REGION_BIT,
                                 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier2);
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

        vkQueueSubmit(vkState.renderQueue, 1, &submitInfo, fences[nextImageID]);

        VkPresentInfoKHR presentInfo{ VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &imageReleaseSemaphore;
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &vkState.swapchain;
        presentInfo.pImageIndices = &nextImageID;

        vkQueuePresentKHR(vkState.renderQueue, &presentInfo);

        glfwPollEvents();
    }

    // TODO: Destroy objects

    glfwDestroyWindow(window);

    glfwTerminate();

    return 0;
}
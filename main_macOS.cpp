#include "vulkan/vk_helpers.h"
#include "vulkan/vk_init.cpp"
#include "vulkan/vk_swapchain.cpp"
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <vector>

VkRenderPass createRenderPass(VkDevice device, VkFormat swapchainFormat){
    VkAttachmentDescription attachments[1]{};
    attachments[0].format = swapchainFormat;
    attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorRef{};
    colorRef.attachment = 0;
    colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpassDescr{};
    subpassDescr.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDescr.colorAttachmentCount = 1;
    subpassDescr.pColorAttachments = &colorRef;

    VkSubpassDependency subpassDeps[2]{};
    subpassDeps[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    subpassDeps[0].dstSubpass = 0;
    subpassDeps[0].srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    subpassDeps[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpassDeps[0].srcAccessMask = 0;
    subpassDeps[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    subpassDeps[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    subpassDeps[1].srcSubpass = 0;
    subpassDeps[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    subpassDeps[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpassDeps[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    subpassDeps[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    subpassDeps[1].dstAccessMask = 0;
    subpassDeps[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    VkRenderPassCreateInfo renderPassCreateInfo{ VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
    renderPassCreateInfo.attachmentCount = 1;
    renderPassCreateInfo.pAttachments = attachments;
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpassDescr;
    renderPassCreateInfo.dependencyCount = 2;
    renderPassCreateInfo.pDependencies = subpassDeps;

    VkRenderPass renderPass{VK_NULL_HANDLE};
    VK_CHECK(vkCreateRenderPass(device, &renderPassCreateInfo, nullptr, &renderPass));

    return renderPass;
}

VkFramebuffer createFramebuffer(VkDevice device, VkRenderPass renderPass, VkImage image, VkFormat format, uint32_t width, uint32_t height){
    VkImageViewCreateInfo imageViewCreateInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
    imageViewCreateInfo.image = image;
    imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageViewCreateInfo.format = format;
    imageViewCreateInfo.components = {VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A};
    imageViewCreateInfo.subresourceRange.layerCount = 1;
    imageViewCreateInfo.subresourceRange.levelCount = 1;
    imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    VkImageView swapchainView{};
    VK_CHECK(vkCreateImageView(device, &imageViewCreateInfo, nullptr, &swapchainView));

    VkFramebufferCreateInfo createInfo{ VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
    createInfo.renderPass = renderPass;
    createInfo.attachmentCount = 1;
    createInfo.pAttachments = &swapchainView;
    createInfo.width = width;
    createInfo.height = height;
    createInfo.layers = 1;

    VkFramebuffer framebuffer{};
    VK_CHECK(vkCreateFramebuffer(device, &createInfo, nullptr, &framebuffer));

    return framebuffer;
}

VkShaderModule createShaderModule(VkDevice device, const char* filename){
    assert(filename);

    FILE* shaderFile{ fopen(filename, "rb") };
    assert(shaderFile);

    fseek(shaderFile, 0l, SEEK_END);
    long endOffset{ ftell(shaderFile) };
    assert(endOffset >= 0);

    size_t codeSize{static_cast<uint32_t>(endOffset)};

    fseek(shaderFile, 0l, SEEK_SET);

    std::vector<uint8_t> shaderCode(codeSize);
    size_t codeReadObjects{fread(shaderCode.data(), codeSize, 1, shaderFile)};
    assert(codeReadObjects == 1);

    VkShaderModuleCreateInfo createInfo{ VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
    createInfo.codeSize = codeSize;
    createInfo.pCode = reinterpret_cast<const uint32_t*>(shaderCode.data());

    VkShaderModule shaderModule{};
    VK_CHECK(vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule));

    fclose(shaderFile);

    return shaderModule;
}

VkPipeline createGraphicsPipeline(VkDevice device, VkRenderPass renderPass, VkViewport viewport){
    VkShaderModule vertexShader{ createShaderModule(device, "shaders/triangle.vs.spv") };
    VkShaderModule fragmentShader{ createShaderModule(device, "shaders/triangle.fs.spv") };

    VkPipelineShaderStageCreateInfo shaderStages[2]{};
    shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shaderStages[0].module = vertexShader;
    shaderStages[0].pName = "main";

    shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shaderStages[1].module = fragmentShader;
    shaderStages[1].pName = "main";

    VkPipelineVertexInputStateCreateInfo vertexState{ VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
    VkPipelineInputAssemblyStateCreateInfo inputAssemblerState{ VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
    inputAssemblerState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkRect2D scissorRect{ {0, 0}, {(uint32_t)viewport.width, (uint32_t)viewport.height} };

    VkPipelineViewportStateCreateInfo viewportState{ VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissorRect;

    VkPipelineRasterizationStateCreateInfo rasterState{ VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
    rasterState.polygonMode = VK_POLYGON_MODE_FILL;
    rasterState.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterState.lineWidth = 1.0f;

    VkPipelineColorBlendAttachmentState blendAttachment{};
    blendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                     VK_COLOR_COMPONENT_B_BIT |VK_COLOR_COMPONENT_A_BIT;

    VkPipelineColorBlendStateCreateInfo blendState{ VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
    blendState.attachmentCount = 1;
    blendState.pAttachments = &blendAttachment;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
    VkPipelineLayout pipelineLayout{};
    VK_CHECK(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout));

    // TODO: use VkPipelineCache
    VkGraphicsPipelineCreateInfo createPipelineInfo{ VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
    createPipelineInfo.stageCount = 2;
    createPipelineInfo.pStages = shaderStages;
    createPipelineInfo.pVertexInputState = &vertexState;
    createPipelineInfo.pInputAssemblyState = &inputAssemblerState;
    createPipelineInfo.pViewportState = &viewportState;
    createPipelineInfo.pRasterizationState = &rasterState;
    createPipelineInfo.pColorBlendState = &blendState;
    createPipelineInfo.layout = pipelineLayout;
    createPipelineInfo.renderPass = renderPass;

    VkPipeline pipeline{};
    VK_CHECK(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &createPipelineInfo, nullptr, &pipeline));

    return pipeline;
}

int main() {
    int glfwInitResult{ glfwInit() };
    assert(glfwInitResult == GLFW_TRUE);

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(1024, 768, "Render Box", nullptr, nullptr);

    int windowWidth{};
    int windowHeight{};
    glfwGetWindowSize(window, &windowWidth, &windowHeight);

    VulkanState vkState{ initializeVulkanState() };

    VulkanSwapchain vkSwapchain{ createSwapchain(window, vkState) };

    VkCommandPoolCreateInfo cmdPoolCreateInfo{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
    cmdPoolCreateInfo.queueFamilyIndex = vkState.renderQueueFamilyID;
    cmdPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    VkCommandPool cmdPool{VK_NULL_HANDLE};
    VK_CHECK(vkCreateCommandPool(vkState.device, &cmdPoolCreateInfo, nullptr, &cmdPool));

    VkCommandBufferAllocateInfo cmdBuffersAllocInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
    cmdBuffersAllocInfo.commandBufferCount = vkSwapchain.images.size();
    cmdBuffersAllocInfo.commandPool = cmdPool;
    cmdBuffersAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    std::vector<VkCommandBuffer> cmdBuffers(vkSwapchain.images.size());
    VK_CHECK(vkAllocateCommandBuffers(vkState.device, &cmdBuffersAllocInfo, cmdBuffers.data()));

    VkSemaphoreCreateInfo semCreateInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
    VkSemaphore imageAcquireSemaphore{ VK_NULL_HANDLE };
    VK_CHECK(vkCreateSemaphore(vkState.device, &semCreateInfo, nullptr, &imageAcquireSemaphore));

    VkSemaphore imageReleaseSemaphore{ VK_NULL_HANDLE };
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
        framebuffers[i] = createFramebuffer(vkState.device, triangleRenderPass, vkSwapchain.images[i],
                                            vkSwapchain.surfaceFormat.format, windowWidth, windowHeight);
    }

    VkViewport viewport{ 0.0f, 0.0f, (float)windowWidth, (float)windowHeight, 0.0f, 1.0f };
    VkPipeline trianglePipeline{ createGraphicsPipeline(vkState.device, triangleRenderPass, viewport) };

    while(!glfwWindowShouldClose(window)){
        uint32_t nextImageID{};
        VkResult acquireRes{vkAcquireNextImageKHR(vkState.device, vkSwapchain.swapchain, -1, imageAcquireSemaphore, VK_NULL_HANDLE, &nextImageID)};
        assert(acquireRes == VK_SUCCESS || acquireRes == VK_SUBOPTIMAL_KHR);

        VK_CHECK(vkWaitForFences(vkState.device, 1, &fences[nextImageID], VK_FALSE, -1));
        VK_CHECK(vkResetFences(vkState.device, 1, &fences[nextImageID]));

        VkCommandBufferBeginInfo cmdBeginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
        cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        VK_CHECK(vkBeginCommandBuffer(cmdBuffers[nextImageID], &cmdBeginInfo));
        {
            VkClearValue clearValue{{{0.2f, 0.3f, 0.4f, 1.0f}}};

            VkRenderPassBeginInfo renderPassBeginInfo{ VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
            renderPassBeginInfo.renderPass = triangleRenderPass;
            renderPassBeginInfo.framebuffer = framebuffers[nextImageID];
            renderPassBeginInfo.renderArea.extent.width = windowWidth;
            renderPassBeginInfo.renderArea.extent.height = windowHeight;
            renderPassBeginInfo.clearValueCount = 1;
            renderPassBeginInfo.pClearValues = &clearValue;

            vkCmdBeginRenderPass(cmdBuffers[nextImageID], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

            vkCmdBindPipeline(cmdBuffers[nextImageID], VK_PIPELINE_BIND_POINT_GRAPHICS, trianglePipeline);
            vkCmdDraw(cmdBuffers[nextImageID], 3, 1, 0, 0);

            vkCmdEndRenderPass(cmdBuffers[nextImageID]);
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
        presentInfo.pSwapchains = &vkSwapchain.swapchain;
        presentInfo.pImageIndices = &nextImageID;

        vkQueuePresentKHR(vkState.renderQueue, &presentInfo);

        glfwPollEvents();
    }

    // TODO: Destroy objects

    glfwDestroyWindow(window);

    glfwTerminate();

    return 0;
}

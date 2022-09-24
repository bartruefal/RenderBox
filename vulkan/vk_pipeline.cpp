#include "vk_helpers.h"
#include <stdio.h>

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

GraphicsPipeline createGraphicsPipeline(VkDevice device, VkRenderPass renderPass, VkViewport viewport, VkPipelineLayout pipelineLayout){
    GraphicsPipeline pipeline{};
    pipeline.vertexShader = createShaderModule(device, "shaders/triangle.vs.spv");
    pipeline.fragmentShader = createShaderModule(device, "shaders/triangle.fs.spv");

    VkPipelineShaderStageCreateInfo shaderStages[2]{};
    shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shaderStages[0].module = pipeline.vertexShader;
    shaderStages[0].pName = "main";

    shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shaderStages[1].module = pipeline.fragmentShader;
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

    VK_CHECK(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &createPipelineInfo, nullptr, &pipeline.pipeline));

    return pipeline;
}

void destroyPipeline(VkDevice device, GraphicsPipeline pipeline){
    vkDestroyPipeline(device, pipeline.pipeline, nullptr);

    vkDestroyShaderModule(device, pipeline.fragmentShader, nullptr);
    vkDestroyShaderModule(device, pipeline.vertexShader, nullptr);
}
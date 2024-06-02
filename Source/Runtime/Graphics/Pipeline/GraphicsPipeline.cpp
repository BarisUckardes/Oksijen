#include "GraphicsPipeline.h"
#include <Runtime/Graphics/Device/GraphicsDevice.h>
#include <Runtime/Graphics/Descriptor/DescriptorSetLayout.h>
#include <Runtime/Graphics/RenderPass/RenderPass.h>

namespace Oksijen
{
    GraphicsPipeline::GraphicsPipeline(
        const GraphicsDevice* pDevice,
        const VkPipelineVertexInputStateCreateInfo& vertexInputState,
        const VkPipelineInputAssemblyStateCreateInfo& inputAssemblyState,
        const VkPipelineTessellationStateCreateInfo* pTesellationState,
        const VkPipelineViewportStateCreateInfo* pViewportState,
        const VkPipelineRasterizationStateCreateInfo& rasterizerState,
        const VkPipelineMultisampleStateCreateInfo& multisampleState,
        const VkPipelineDepthStencilStateCreateInfo& depthStencilState,
        const VkPipelineColorBlendStateCreateInfo& blendingState,
        const VkPipelineDynamicStateCreateInfo* pDynamicState,
        const Shader** ppShaders, const unsigned int shaderCount,
        const DescriptorSetLayout** ppDescriptorLayouts, const unsigned int descriptorLayoutCount,
        const RenderPass* pRenderPass,
        const Pipeline* pBasePipeline, const unsigned int basePipelineIndex,
        const void* pNext) 
        : Pipeline(pDevice,VK_PIPELINE_BIND_POINT_GRAPHICS),mLogicalDevice(pDevice->GetLogicalDevice())
    {
        //Create pipeline layout
        VkDescriptorSetLayout descriptorSetLayouts[128];
        for (unsigned int i = 0; i < descriptorLayoutCount; i++)
        {
            descriptorSetLayouts[i] = ppDescriptorLayouts[i]->GetLayout();
        }

        VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = descriptorLayoutCount;
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts;
        pipelineLayoutInfo.pPushConstantRanges = nullptr;
        pipelineLayoutInfo.pushConstantRangeCount = 0;
        pipelineLayoutInfo.pNext = nullptr;

        VkPipelineLayout layout = VK_NULL_HANDLE;
        DEV_ASSERT(vkCreatePipelineLayout(mLogicalDevice, &pipelineLayoutInfo, nullptr, &layout) == VK_SUCCESS, "GraphicsPipeline", "Failed to create pipeline layout");

        //Create shader stages
        VkPipelineShaderStageCreateInfo shaderStageInformations[16];
        for (unsigned int i = 0; i < shaderCount; i++)
        {
            const Shader* pShader = ppShaders[i];

            VkPipelineShaderStageCreateInfo stageInfo = {};
            stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            stageInfo.flags = VkShaderStageFlags();
            stageInfo.module = pShader->GetModule();
            stageInfo.pName = pShader->GetEntryPoint().c_str();
            stageInfo.stage = (VkShaderStageFlagBits)pShader->GetStage();
            stageInfo.pNext = nullptr;

            shaderStageInformations[i] = stageInfo;
        }

        //Create rendering info
        VkGraphicsPipelineCreateInfo pipelineInfo = {};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.pVertexInputState = &vertexInputState;
        pipelineInfo.pInputAssemblyState = &inputAssemblyState;
        pipelineInfo.pTessellationState = pTesellationState;
        pipelineInfo.pViewportState = pViewportState;
        pipelineInfo.pRasterizationState = &rasterizerState;
        pipelineInfo.pMultisampleState = &multisampleState;
        pipelineInfo.pDepthStencilState = &depthStencilState;
        pipelineInfo.pColorBlendState = &blendingState;
        pipelineInfo.pDynamicState = pDynamicState;
        pipelineInfo.stageCount = shaderCount;
        pipelineInfo.pStages = shaderStageInformations;
        pipelineInfo.layout = layout;
        pipelineInfo.basePipelineHandle = pBasePipeline != nullptr ? pBasePipeline->GetPipeline() : nullptr;
        pipelineInfo.basePipelineIndex = basePipelineIndex;
        pipelineInfo.subpass = 0;
        pipelineInfo.renderPass = pRenderPass != nullptr ? pRenderPass->GetRenderPass() : VK_NULL_HANDLE;
        pipelineInfo.pNext = pNext;

        VkPipeline pipeline = VK_NULL_HANDLE;
        DEV_ASSERT(vkCreateGraphicsPipelines(mLogicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) == VK_SUCCESS, "GraphicsPipeline", "Failed to create pipeline");

        SetPipelineProperties(pipeline, layout);

    }
    GraphicsPipeline::~GraphicsPipeline()
    {
        
    }
}
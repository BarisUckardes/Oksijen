#include "ComputePipeline.h"
#include <Runtime/Graphics/Device/GraphicsDevice.h>
#include <Runtime/Graphics/Descriptor/DescriptorSetLayout.h>
#include <Runtime/Graphics/Shader/Shader.h>

namespace Oksijen
{
	ComputePipeline::ComputePipeline(
		const GraphicsDevice* pDevice,
		const VkPipelineCreateFlags flags,
		const Shader* pShader,
		const DescriptorSetLayout** ppSetLayouts, const unsigned int setLayoutCount,
		const Pipeline* pBasePipeline, const unsigned int basePipelineIndex) : Pipeline(pDevice,VK_PIPELINE_BIND_POINT_COMPUTE),mLogicalDevice(pDevice->GetLogicalDevice())
	{
		//Create shader stage info
		VkPipelineShaderStageCreateInfo stageInfo = {};
		stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		stageInfo.flags = VkPipelineShaderStageCreateFlags();
		stageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
		stageInfo.module = pShader->GetModule();
		stageInfo.pName = pShader->GetEntryPoint().c_str();
		stageInfo.pSpecializationInfo = nullptr;
		stageInfo.pNext = nullptr;

		//Create layout
		VkDescriptorSetLayout setLayouts[32];
		for (unsigned int i = 0; i < setLayoutCount; i++)
		{
			setLayouts[i] = ppSetLayouts[i]->GetLayout();
		}

		VkPipelineLayoutCreateInfo layoutInfo = {};
		layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		layoutInfo.setLayoutCount = setLayoutCount;
		layoutInfo.pSetLayouts = setLayouts;
		layoutInfo.pushConstantRangeCount = 0;
		layoutInfo.pPushConstantRanges = nullptr;
		layoutInfo.pNext = nullptr;

		VkPipelineLayout layout = VK_NULL_HANDLE;
		DEV_ASSERT(vkCreatePipelineLayout(mLogicalDevice, &layoutInfo, nullptr, &layout) == VK_SUCCESS, "ComputePipeline", "Failed to create compute pipeline layout");

		VkComputePipelineCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		info.flags = flags;
		info.layout = layout;
		info.basePipelineHandle = pBasePipeline != nullptr ? pBasePipeline->GetPipeline() : VK_NULL_HANDLE;
		info.basePipelineIndex = basePipelineIndex;
		info.stage = stageInfo;
		info.pNext = nullptr;

		VkPipeline pipeline = VK_NULL_HANDLE;
		DEV_ASSERT(vkCreateComputePipelines(mLogicalDevice, VK_NULL_HANDLE, 1, &info, nullptr, &pipeline) == VK_SUCCESS, "ComputePipeline", "Failed to create compute pipeline");

		SetPipelineProperties(pipeline, layout);
	}
	ComputePipeline::~ComputePipeline()
	{

	}
}
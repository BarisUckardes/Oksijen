#pragma once
#include <Runtime/Graphics/Pipeline/Pipeline.h>

namespace Oksijen
{
	class GraphicsDevice;
	class Shader;
	class DescriptorSetLayout;
	class RenderPass;
	class RUNTIME_API GraphicsPipeline final : public Pipeline
	{
	public:
		GraphicsPipeline(
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
			const Shader** ppShaders,const unsigned int shaderCount,
			const DescriptorSetLayout** ppDescriptorLayouts,const unsigned int descriptorLayoutCount,
			const RenderPass* pRenderPass,
			const Pipeline* pBasePipeline,const unsigned int basePipelineIndex,
			const void* pNext);
		~GraphicsPipeline();

	private:
		const VkDevice mLogicalDevice;
	};
}
#pragma once
#include <Runtime/Graphics/Pipeline/Pipeline.h>

namespace Oksijen
{
	class GraphicsDevice;
	class DescriptorSetLayout;
	class Shader;
	class RUNTIME_API ComputePipeline final : public Pipeline
	{
	public:
		ComputePipeline(
			const GraphicsDevice* pDevice,
			const VkPipelineCreateFlags flags,
			const Shader* pShader,
			const DescriptorSetLayout** ppSetLayouts,const unsigned int setLayoutCount,
			const Pipeline* pBasePipeline,const unsigned int basePipelineIndex);
		~ComputePipeline();

	private:
		const VkDevice mLogicalDevice;

	};
}
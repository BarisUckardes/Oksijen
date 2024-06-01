#pragma once
#include <Runtime/Core/Core.h>
#include <vulkan/vulkan.h>

namespace Oksijen
{
	class GraphicsDevice;
	class Sampler;
	class RUNTIME_API DescriptorSetLayout final
	{
	public:
		DescriptorSetLayout(
			const GraphicsDevice* pDevice,
			const unsigned int* pBindings,
			const VkDescriptorType* pDescriptorTypes,
			const unsigned int* pDescriptorCounts,
			const VkShaderStageFlags* pStageFlags,
			const unsigned char bindingCount);
		~DescriptorSetLayout();

		FORCEINLINE VkDescriptorSetLayout GetLayout() const noexcept { return mLayout; }
	private:
		const VkDevice mLogicalDevice;
		VkDescriptorSetLayout mLayout;
	};
}
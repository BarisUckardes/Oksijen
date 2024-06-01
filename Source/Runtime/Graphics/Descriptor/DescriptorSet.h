#pragma once
#include <Runtime/Core/Core.h>
#include <vulkan/vulkan.h>

namespace Oksijen
{
	class GraphicsDevice;
	class DescriptorPool;
	class DescriptorSetLayout;
	class RUNTIME_API DescriptorSet final
	{
	public:
		DescriptorSet(const GraphicsDevice* pDevice,const DescriptorPool* pPool,const DescriptorSetLayout* pLayout);
		~DescriptorSet();

		FORCEINLINE VkDescriptorSet GetSet() const noexcept { return mSet; }
	private:
		const VkDevice mLogicalDevice;
		const VkDescriptorPool mPool;
		const VkDescriptorSetLayout mLayout;
		VkDescriptorSet mSet;
	};
}
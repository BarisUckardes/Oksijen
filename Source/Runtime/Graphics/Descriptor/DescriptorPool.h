#pragma once
#include <Runtime/Core/Core.h>
#include <vulkan/vulkan.h>

namespace Oksijen
{
	class GraphicsDevice;
	class RUNTIME_API DescriptorPool final
	{
	public:
		DescriptorPool(
			const GraphicsDevice* pDevice,
			const VkDescriptorPoolCreateFlags flags,
			const unsigned int maxSets,
			const VkDescriptorType* pDescriptorTypes,const unsigned int* pDescriptorCounts,
			const unsigned int descriptorTypeCount);
		~DescriptorPool();

		FORCEINLINE VkDescriptorPool GetPool() const noexcept { return mPool; }
	private:
		const VkDevice mLogicalDevice;
		VkDescriptorPool mPool;
	};
}
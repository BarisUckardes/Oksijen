#include "DescriptorSet.h"
#include <Runtime/Graphics/Device/GraphicsDevice.h>
#include <Runtime/Graphics/Descriptor/DescriptorPool.h>
#include <Runtime/Graphics/Descriptor/DescriptorSetLayout.h>

namespace Oksijen
{
	DescriptorSet::DescriptorSet(const GraphicsDevice* pDevice, const DescriptorPool* pPool, const DescriptorSetLayout* pLayout) : mLogicalDevice(pDevice->GetLogicalDevice()),mPool(pPool->GetPool()),mLayout(pLayout->GetLayout()), mSet(VK_NULL_HANDLE)
	{
		VkDescriptorSetAllocateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		info.descriptorPool = mPool;
		info.pSetLayouts = &mLayout;
		info.descriptorSetCount = 1;
		info.pNext = nullptr;

		DEV_ASSERT(vkAllocateDescriptorSets(mLogicalDevice, &info, &mSet) == VK_SUCCESS, "DescriptorSet", "Failed to allocate descriptor set");
	}
	DescriptorSet::~DescriptorSet()
	{
		vkFreeDescriptorSets(mLogicalDevice, mPool, 1, &mSet);
	}
}
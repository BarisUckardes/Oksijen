#include "DescriptorSetLayout.h"
#include <Runtime/Graphics/Device/GraphicsDevice.h>
#include <Runtime/Graphics/Sampler/Sampler.h>

namespace Oksijen
{
	
	DescriptorSetLayout::DescriptorSetLayout(
		const GraphicsDevice* pDevice,
		const unsigned int* pBindings,
		const VkDescriptorType* pDescriptorTypes,
		const unsigned int* pDescriptorCounts,
		const VkShaderStageFlags* pStageFlags,
		const unsigned char bindingCount) : mLogicalDevice(pDevice->GetLogicalDevice()),mLayout(VK_NULL_HANDLE)
	{

		//Collect bindings
		VkDescriptorSetLayoutBinding bindings[128];
		for (unsigned int i = 0; i < bindingCount; i++)
		{
			VkDescriptorSetLayoutBinding binding = {};
			binding.binding = pBindings[i];
			binding.descriptorType = pDescriptorTypes[i];
			binding.descriptorCount = pDescriptorCounts[i];
			binding.stageFlags = pStageFlags[i];
			binding.pImmutableSamplers = nullptr;

			bindings[i] = binding;
		}
		
		//Create layout
		VkDescriptorSetLayoutCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		createInfo.pBindings = bindings;
		createInfo.bindingCount = bindingCount;
		createInfo.flags = VkDescriptorSetLayoutCreateFlags();
		createInfo.pNext = nullptr;

		DEV_ASSERT(vkCreateDescriptorSetLayout(mLogicalDevice, &createInfo, nullptr, &mLayout) == VK_SUCCESS, "VulkanDescriptorLayout", "Failed to create layout");
	}
	DescriptorSetLayout::~DescriptorSetLayout()
	{
		vkDestroyDescriptorSetLayout(mLogicalDevice, mLayout, nullptr);
	}
}
#include "Texture.h"
#include <Runtime/Graphics/Device/GraphicsDevice.h>
#include <Runtime/Graphics/Memory/GraphicsMemory.h>

namespace Oksijen
{
	Texture::Texture(const GraphicsDevice* pDevice,
		GraphicsMemory* pMemory,
		const VkImageType type,
		const VkFormat format,
		const unsigned int width, const unsigned int height, const unsigned int depth,
		const unsigned char mipLevels, const unsigned char arrayLevels,
		const VkSampleCountFlagBits samples,
		const VkImageTiling tiling,
		const VkImageUsageFlags usageFlags,
		const VkSharingMode sharingMode,
		const VkImageLayout imageLayout) :
		mLogicalDevice(pDevice->GetLogicalDevice()),mImage(VK_NULL_HANDLE),mTargetMemory(pMemory),mMemoryOffset(uint64_max),mAlignedMemoryOffset(uint64_max),mMipLevels(mipLevels),mArrayLevels(arrayLevels),mSwapchainImage(false)
	{

		//Create image
		VkImageCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		info.imageType = type;
		info.format = format;
		info.extent = { width,height,depth };
		info.mipLevels = mipLevels;
		info.arrayLayers = arrayLevels;
		info.samples = samples;
		info.tiling = tiling;
		info.usage = usageFlags;
		info.sharingMode = sharingMode;
		info.initialLayout = imageLayout;
		info.pNext = nullptr;

		DEV_ASSERT(vkCreateImage(mLogicalDevice, &info, nullptr, &mImage) == VK_SUCCESS,"Texture","Failed to create texture");

		//Bind memory
		VkMemoryRequirements requirements = {};
		vkGetImageMemoryRequirements(mLogicalDevice, mImage, &requirements);

		const unsigned long long memoryOffset = pMemory->Allocate(requirements.size + requirements.alignment);
		const unsigned long long memoryAlignedOffset = memoryOffset + (memoryOffset == 0 ? 0 : (requirements.alignment - (memoryOffset % requirements.alignment)));

		DEV_ASSERT(vkBindImageMemory(mLogicalDevice, mImage, pMemory->GetMemory(), memoryAlignedOffset) == VK_SUCCESS, "VulkanTexture", "Failed to bind the texture memory!");

		mMemoryOffset = memoryOffset;
		mAlignedMemoryOffset = memoryAlignedOffset;
	}
	Texture::Texture(const VkImage swapchainImage) : mImage(swapchainImage),mLogicalDevice(VK_NULL_HANDLE),mTargetMemory(nullptr), mMemoryOffset(uint64_max), mAlignedMemoryOffset(uint64_max),mMipLevels(1),mArrayLevels(1),mSwapchainImage(true)
	{

	}
	Texture::~Texture()
	{
		if (mSwapchainImage)
			return;

		vkDestroyImage(mLogicalDevice, mImage, nullptr);
	}
}
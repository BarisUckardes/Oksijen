#include "TextureView.h"
#include <Runtime/Graphics/Device/GraphicsDevice.h>
#include <Runtime/Graphics/Texture/Texture.h>

namespace Oksijen
{
	TextureView::TextureView(const GraphicsDevice* pDevice, const Texture* pTexture, const unsigned int mipIndex, const unsigned int arrayIndex, const VkImageViewType viewType, const VkFormat format, const VkComponentMapping mapping, const VkImageAspectFlags aspectMask) :
		mLogicalDevice(pDevice->GetLogicalDevice())
	{
		VkImageViewCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		info.image = pTexture->GetImage();
		info.viewType = viewType;
		info.subresourceRange.aspectMask = aspectMask;
		info.subresourceRange.baseMipLevel = mipIndex;
		info.subresourceRange.baseArrayLayer = arrayIndex;
		info.subresourceRange.layerCount = pTexture->GetArrayLevels();
		info.subresourceRange.levelCount = pTexture->GetMipLevels();
		info.components = mapping;
		info.format = format;
		info.pNext = nullptr;

		DEV_ASSERT(vkCreateImageView(mLogicalDevice, &info, nullptr, &mView) == VK_SUCCESS, "TextureView", "Failed to create image view");
	}
	TextureView::~TextureView()
	{
		vkDestroyImageView(mLogicalDevice, mView, nullptr);
	}
}
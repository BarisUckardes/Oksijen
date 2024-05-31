#include "Swapchain.h"
#include <Runtime/Graphics/Surface/Surface.h>
#include <Runtime/Graphics/Adapter/GraphicsAdapter.h>
#include <Runtime/Graphics/Queue/GraphicsQueue.h>

namespace Oksijen
{
	
	Swapchain::Swapchain(const Surface* pSurface,GraphicsQueue* pQueue,const GraphicsDevice* pDevice, const unsigned int bufferCount, const unsigned int width, const unsigned int height, const VkPresentModeKHR presentMode, const VkFormat format, const VkColorSpaceKHR formatColorspace, const VkImageUsageFlags imageUsageFlags) :
		mLogicalDevice(pDevice->GetLogicalDevice()),mTargetQueue(pQueue), mSwapchain(VK_NULL_HANDLE), mBufferCount(bufferCount), mWidth(width), mHeight(height), mPresentMode(presentMode), mFormat(format), mFormatColorSpace(formatColorspace), mImageUsageFlags(imageUsageFlags), mSurface(pSurface->GetSurface())
	{
		ResizeInternal();

	}
	Swapchain::~Swapchain()
	{
		vkDestroySwapchainKHR(mLogicalDevice, mSwapchain, nullptr);
	}
	void Swapchain::Resize(const unsigned int width, const unsigned int height)
	{
		//Cleanup former images/buffers
		for (const VkImageView view : mViews)
		{
			vkDestroyImageView(mLogicalDevice, view, nullptr);
		}
		for (Texture* pTexture : mTextures)
		{
			delete pTexture;
		}
		mTextures.clear();
		mViews.clear();

		//Set new properties
		mWidth = width;
		mHeight = height;

		//Resize internal
		ResizeInternal();
	}
	unsigned int Swapchain::AcquireImageIndex(const Fence* pFence, const Semaphore* pSemaphore)
	{
		unsigned int imageIndex = 0;
		DEV_ASSERT(vkAcquireNextImageKHR(mLogicalDevice, mSwapchain, uint64_max, pSemaphore != nullptr ? pSemaphore->GetSemaphore() : nullptr, pFence != nullptr ? pFence->GetFence() : nullptr, &imageIndex) == VK_SUCCESS, "Swapchain", "Failed to get next image");
		return imageIndex;
	}
	void Swapchain::Present(const unsigned int index)
	{
		VkPresentInfoKHR info = {};
		info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		info.waitSemaphoreCount = 0;
		info.pWaitSemaphores = nullptr;
		info.swapchainCount = 1;
		info.pSwapchains = &mSwapchain;
		info.pImageIndices = &index;
		info.pResults = nullptr;

		if (vkQueuePresentKHR(mTargetQueue->GetQueue(), &info) != VK_SUCCESS)
		{
			DEV_LOG("VulkanSwapchain", "Failed to present to the queue");
			return;
		}
	}
	void Swapchain::ResizeInternal()
	{
		VkSwapchainCreateInfoKHR swapchainInfo = {};
		swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapchainInfo.surface = mSurface;
		swapchainInfo.imageFormat = mFormat;
		swapchainInfo.imageColorSpace = mFormatColorSpace;
		swapchainInfo.imageExtent = { mWidth,mHeight };
		swapchainInfo.imageArrayLayers = 1;
		swapchainInfo.imageUsage = mImageUsageFlags;
		swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		swapchainInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		swapchainInfo.presentMode = mPresentMode;
		swapchainInfo.clipped = VK_FALSE;
		swapchainInfo.oldSwapchain = VK_NULL_HANDLE;
		swapchainInfo.minImageCount = mBufferCount;

		const unsigned int queueFamilyIndex = mTargetQueue->GetFamilyIndex();
		swapchainInfo.queueFamilyIndexCount = 1;
		swapchainInfo.pQueueFamilyIndices = &queueFamilyIndex;
		swapchainInfo.flags = VkSwapchainCreateFlagsKHR();
		swapchainInfo.pNext = nullptr;

		DEV_ASSERT(vkCreateSwapchainKHR(mLogicalDevice, &swapchainInfo, nullptr, &mSwapchain) == VK_SUCCESS, "Swapchain", "Failed to create swapchain");

		//Get textures
		unsigned int swapchainImageCount = mBufferCount;
		std::vector<VkImage> images(swapchainImageCount);
		DEV_ASSERT(vkGetSwapchainImagesKHR(mLogicalDevice, mSwapchain, &swapchainImageCount, images.data()) == VK_SUCCESS, "Swapchain", "Failed to get swapchain images");
		for (const VkImage image : images)
		{
			mTextures.push_back(new Texture(image));
		}

		//Create views
		for (const VkImage image : images)
		{
			VkImageViewCreateInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			info.viewType = VK_IMAGE_VIEW_TYPE_2D;
			info.image = image;
			info.format = mFormat;
			info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			info.subresourceRange.baseArrayLayer = 0;
			info.subresourceRange.baseMipLevel = 0;
			info.subresourceRange.layerCount = 1;
			info.subresourceRange.levelCount = 1;
			info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			info.flags = VkImageViewCreateFlags();
			info.pNext = nullptr;

			VkImageView view = VK_NULL_HANDLE;
			DEV_ASSERT(vkCreateImageView(mLogicalDevice, &info, nullptr, &view) == VK_SUCCESS, "Swapchain", "Failed to create image view");
			mViews.push_back(view);
		}
	}
}
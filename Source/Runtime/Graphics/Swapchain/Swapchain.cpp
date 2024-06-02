#include "Swapchain.h"
#include <Runtime/Graphics/Surface/Surface.h>
#include <Runtime/Graphics/Adapter/GraphicsAdapter.h>
#include <Runtime/Graphics/Queue/GraphicsQueue.h>

namespace Oksijen
{
	
	Swapchain::Swapchain(const Surface* pSurface,GraphicsQueue* pQueue,const GraphicsDevice* pDevice, const unsigned int bufferCount, const unsigned int width, const unsigned int height,const unsigned int arrayLevels, const VkPresentModeKHR presentMode, const VkFormat format, const VkColorSpaceKHR formatColorspace, const VkImageUsageFlags imageUsageFlags) :
		mLogicalDevice(pDevice->GetLogicalDevice()),mTargetQueue(pQueue), mSwapchain(VK_NULL_HANDLE), mBufferCount(bufferCount), mWidth(width), mHeight(height),mArrayLevels(arrayLevels), mPresentMode(presentMode), mFormat(format), mFormatColorSpace(formatColorspace), mImageUsageFlags(imageUsageFlags), mSurface(pSurface->GetSurface())
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
		for (Texture* pTexture : mTextures)
		{
			delete pTexture;
		}
		mTextures.clear();

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
		swapchainInfo.imageArrayLayers = mArrayLevels;
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
			mTextures.push_back(new Texture(image,mWidth,mHeight,mArrayLevels));
		}
	}
}
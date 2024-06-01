#pragma once
#include <Runtime/Core/Core.h>
#include <vulkan/vulkan.h>
#include <vector>

namespace Oksijen
{
	class Surface;
	class GraphicsAdapter;
	class GraphicsDevice;
	class GraphicsQueue;
	class Semaphore;
	class Fence;
	class Texture;
	class RUNTIME_API Swapchain final
	{
		friend class GraphicsDevice;
	public:
		Swapchain(const Surface* pSurface,GraphicsQueue* pQueue,const GraphicsDevice* pDevice,const unsigned int bufferCount,const unsigned int width,const unsigned int height,const unsigned int arrayLevels,const VkPresentModeKHR presentMode,const VkFormat format,const VkColorSpaceKHR formatColorspace,const VkImageUsageFlags imageUsageFlags);
		~Swapchain();

		FORCEINLINE Texture* GetTexture(const unsigned int index) { return mTextures[index]; }
		FORCEINLINE VkImageView GetImageView(const unsigned int index) { return VK_NULL_HANDLE; }

		void Resize(const unsigned int width, const unsigned int height);
		unsigned int AcquireImageIndex(const Fence* pFence,const Semaphore* pSemaphore);
		void Present(const unsigned int index);
	private:
		void ResizeInternal();
	private:
		const VkDevice mLogicalDevice;
		const VkSurfaceKHR mSurface;
		const unsigned int mBufferCount;
		const VkPresentModeKHR mPresentMode;
		const VkFormat mFormat;
		const VkColorSpaceKHR mFormatColorSpace;
		const VkImageUsageFlags mImageUsageFlags;
		const unsigned int mArrayLevels;
		GraphicsQueue* mTargetQueue;
		VkSwapchainKHR mSwapchain;
		unsigned int mWidth;
		unsigned int mHeight;
		std::vector<Texture*> mTextures;

	};
}
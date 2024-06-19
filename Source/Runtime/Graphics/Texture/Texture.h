#pragma once
#include <Runtime/Core/Core.h>
#include <vulkan/vulkan.h>

namespace Oksijen
{
	class GraphicsDevice;
	class GraphicsMemory;
	class RUNTIME_API Texture final
	{
	public:
		Texture(
			const GraphicsDevice* pDevice,
			GraphicsMemory* pMemory,
			const VkImageType type,
			const VkFormat format,
			const unsigned int width,const unsigned int height,const unsigned int depth,
			const unsigned char mipLevels,const unsigned char arrayLevels,
			const VkSampleCountFlagBits samples,
			const VkImageTiling tiling,
			const VkImageUsageFlags usageFlags,
			const VkSharingMode sharingMode,
			const VkImageLayout imageLayout);
		Texture(const VkImage swapchainImage,const unsigned int width,const unsigned int height,const unsigned int arrayLevels);
		~Texture();

		FORCEINLINE VkFormat GetFormat() const noexcept { return mFormat; }
		FORCEINLINE unsigned int GetWidth() const noexcept { return mWidth; }
		FORCEINLINE unsigned int GetHeight() const noexcept { return mHeight; }
		FORCEINLINE unsigned int GetDepth() const noexcept { return mDepth; }
		FORCEINLINE unsigned char GetMipLevels() const noexcept { return mMipLevels; }
		FORCEINLINE unsigned char GetArrayLevels() const noexcept { return mArrayLevels; }
		FORCEINLINE unsigned long long GetMemoryOffset() const noexcept { return mMemoryOffset; }
		FORCEINLINE unsigned int long long GetAlignedMemoryOffset() const noexcept { return mAlignedMemoryOffset; }
		FORCEINLINE VkImage GetImage() const noexcept { return mImage; }
	private:
		const VkDevice mLogicalDevice;
		const VkFormat mFormat;
		const unsigned int mWidth;
		const unsigned int mHeight;
		const unsigned int mDepth;
		const unsigned char mMipLevels;
		const unsigned char mArrayLevels;
		const bool mSwapchainImage;
		GraphicsMemory* mTargetMemory;
		unsigned long long mMemoryOffset;
		unsigned long long mAlignedMemoryOffset;
		VkImage mImage;
	};
}
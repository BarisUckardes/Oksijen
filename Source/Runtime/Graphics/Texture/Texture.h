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
		Texture(const VkImage swapchainImage);
		~Texture();

		FORCEINLINE VkImage GetImage() const noexcept { return mImage; }
		FORCEINLINE unsigned long long GetMemoryOffset() const noexcept { return mMemoryOffset; }
		FORCEINLINE unsigned int long long GetAlignedMemoryOffset() const noexcept { return mAlignedMemoryOffset; }
		FORCEINLINE unsigned char GetMipLevels() const noexcept { return mMipLevels; }
		FORCEINLINE unsigned char GetArrayLevels() const noexcept { return mArrayLevels; }
	private:
		const VkDevice mLogicalDevice;
		const unsigned char mMipLevels;
		const unsigned char mArrayLevels;
		const bool mSwapchainImage;
		GraphicsMemory* mTargetMemory;
		unsigned long long mMemoryOffset;
		unsigned long long mAlignedMemoryOffset;
		VkImage mImage;
	};
}
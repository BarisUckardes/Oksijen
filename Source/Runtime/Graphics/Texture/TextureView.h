#pragma once
#include <Runtime/Core/Core.h>
#include <vulkan/vulkan.h>

namespace Oksijen
{
	class GraphicsDevice;
	class Texture;
	class RUNTIME_API TextureView final
	{
	public:
		TextureView(const GraphicsDevice* pDevice, const Texture* pTexture,const unsigned char mipIndex,const unsigned char arrayIndex,const VkImageViewType viewType,const VkFormat format,const VkComponentMapping mapping,const VkImageAspectFlags aspectMask);
		~TextureView();

		FORCEINLINE const Texture* GetTexture() const noexcept { return mTexture; }
		FORCEINLINE VkImageView GetView() const noexcept { return mView; }
		FORCEINLINE unsigned char GetMipIndex() const noexcept { return mMipIndex; }
		FORCEINLINE unsigned char GetArrayIndex() const noexcept { return mArrayIndex; }
	private:
		const VkDevice mLogicalDevice;
		const Texture* mTexture;
		const unsigned char mMipIndex;
		const unsigned char mArrayIndex;
		VkImageView mView;
	};
}
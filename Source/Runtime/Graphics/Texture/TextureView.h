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
		TextureView(const GraphicsDevice* pDevice, const Texture* pTexture,const unsigned int mipIndex,const unsigned int arrayIndex,const VkImageViewType viewType,const VkFormat format,const VkComponentMapping mapping,const VkImageAspectFlags aspectMask);
		~TextureView();

		FORCEINLINE VkImageView GetView() const noexcept { return mView; }
	private:
		const VkDevice mLogicalDevice;
		VkImageView mView;
	};
}
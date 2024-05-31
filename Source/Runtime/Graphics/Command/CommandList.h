#pragma once
#include <Runtime/Core/Core.h>
#include <vulkan/vulkan.h>

namespace Oksijen
{
	class GraphicsDevice;
	class CommandPool;
	class Texture;
	class RUNTIME_API CommandList final
	{
	public:
		CommandList(const GraphicsDevice* pDevice, const CommandPool* pPool);
		~CommandList();

		FORCEINLINE VkCommandBuffer GetCmdBuffer() const noexcept { return mCmdBuffer; }

		void Begin();
		void End();

		void SetPipelineTextureBarrier(
			const Texture* pTexture,
			const VkImageLayout oldImageLayout,const VkImageLayout newImageLayout,
			const VkQueueFlags srcQueue,const VkQueueFlags dstQueue,
			const VkImageAspectFlags imageAspectMask,const unsigned char mipIndex,const unsigned char arrayIndex,
			const VkAccessFlags srcAccessMask,const VkAccessFlags dstAccessMask,
			const VkPipelineStageFlags srcPipelineStageMask,const VkPipelineStageFlags dstPipelineStageMask);

		void BeginDynamicRendering(
			const VkRenderingAttachmentInfo* pColorAttachments,const unsigned char colorAttachmentCount,
			const VkRenderingAttachmentInfo* pDepthAttachment,
			const VkRenderingAttachmentInfo* pStencilAttachment,
			const int x,const int y,const unsigned int width,const unsigned height);
		void EndDynamicRendering();
	private:
		const VkDevice mLogicalDevice;
		const VkCommandPool mPool;
		const GraphicsDevice* mDevice;
		VkCommandBuffer mCmdBuffer;
	};
}
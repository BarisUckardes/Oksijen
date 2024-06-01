#pragma once
#include <Runtime/Core/Core.h>
#include <vulkan/vulkan.h>
#include <vector>

namespace Oksijen
{
	class GraphicsDevice;
	class CommandPool;
	class Texture;
	class TextureView;
	class GraphicsBuffer;
	class Pipeline;
	class DescriptorSet;
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
			const VkPipelineStageFlags srcPipelineStageMask,const VkPipelineStageFlags dstPipelineStageMask,
			const VkDependencyFlags dependencies);

		void SetPipelineTextureBarriers(
			const Texture** ppTextures,
			const VkImageLayout* pOldImageLayouts, const VkImageLayout* pNewImageLayouts,
			const VkQueueFlags* pSrcQueues, const VkQueueFlags* pDstQueues,
			const VkImageAspectFlags* pImageAspectMasks, const unsigned char* pMipIndexes, const unsigned char* pArrayIndexes,
			const VkAccessFlags* pSrcAccessMasks, const VkAccessFlags* pDstAccessMasks,
			const VkPipelineStageFlags srcPipelineStageMask, const VkPipelineStageFlags dstPipelineStageMask,
			const VkDependencyFlags dependencies,
			const unsigned int count);

		void SetPipelineMemoryBarrier(
			const VkAccessFlags srcAccessMask,const VkAccessFlags dstAccessMask,
			const VkPipelineStageFlags srcPipelineStageMask, const VkPipelineStageFlags dstPipelineStageMask,
			const VkDependencyFlags dependencies);

		void SetPipelineMemoryBarriers(
			const VkAccessFlags* pSrcAccessMasks, const VkAccessFlags* pDstAccessMasks,
			const VkPipelineStageFlags srcPipelineStageMask, const VkPipelineStageFlags dstPipelineStageMask,
			const VkDependencyFlags dependencies,
			const unsigned int count
		);

		void SetPipelineBufferBarrier(
			const GraphicsBuffer* pBuffer,
			const VkAccessFlags srcAccessMask, const VkAccessFlags dstAccessMask,
			const VkQueueFlags srcQueue, const VkQueueFlags dstQueue,
			const unsigned long long offset,const unsigned long long size,
			const VkPipelineStageFlags srcPipelineStageMask, const VkPipelineStageFlags dstPipelineStageMask,
			const VkDependencyFlags dependencies);

		void SetPipelineBufferBarriers(const GraphicsBuffer** ppBuffers,
			const VkAccessFlags* pSrcAccessMasks, const VkAccessFlags* pDstAccessMasks,
			const VkQueueFlags* pSrcQueues, const VkQueueFlags* pDstQueues,
			const unsigned long long* pOffsets, const unsigned long long* pSizes,
			const VkPipelineStageFlags srcPipelineStageMask, const VkPipelineStageFlags dstPipelineStageMask,
			const VkDependencyFlags dependencies,
			const unsigned int count);

		void SetPipelineBarriers(
			const VkAccessFlags* pMemorySrcAccessMask, const VkAccessFlags* pMemoryDstAccessMask,

			const GraphicsBuffer** ppBuffers,
			const VkAccessFlags* pBufferSrcAccessMasks, const VkAccessFlags* pBufferDstAccessMasks,
			const VkQueueFlags* pBufferSrcQueues, const VkQueueFlags* pBufferDstQueues,
			const unsigned long long* pBufferOffsets, const unsigned long long* pBufferSizes,

			const Texture** ppTextures,
			const VkImageLayout* pTextureOldImageLayouts, const VkImageLayout* pTextureNewImageLayouts,
			const VkQueueFlags* pTextureSrcQueues, const VkQueueFlags* pTextureDstQueues,
			const VkImageAspectFlags* pTextureAspectMasks, const unsigned char* pTextureMipIndexes, const unsigned char* pTextureArrayIndexes,
			const VkAccessFlags* pTextureSrcAccessMasks, const VkAccessFlags* pTextureDstAccessMasks,

			const VkPipelineStageFlags srcPipelineStageMask, const VkPipelineStageFlags dstPipelineStageMask,
			const VkDependencyFlags dependencies,
			const unsigned int memoryCount,const unsigned int bufferCount,const unsigned int textureCount
		);
		void AddDynamicRenderingColorAttachment(
			const TextureView* pAttachmentView,
			const VkImageLayout attachmentLayout,
			const VkResolveModeFlags resolveFlags,
			const TextureView* pResolveAttachmentView,
			const VkImageLayout resolveAttachmentLayout,
			const VkAttachmentLoadOp attachmentLoadOp,
			const VkAttachmentStoreOp attachmentStoreOp,
			const VkClearValue attachmentClearValue);
		void SetDynamicRenderingDepthAttachment(
			const TextureView* pAttachmentView,
			const VkImageLayout attachmentLayout,
			const VkResolveModeFlags resolveFlags,
			const TextureView* pResolveAttachmentView,
			const VkImageLayout resolveAttachmentLayout,
			const VkAttachmentLoadOp attachmentLoadOp,
			const VkAttachmentStoreOp attachmentStoreOp,
			const VkClearValue attachmentClearValue);
		void SetDynamicRenderingStencilAttachment(
			const TextureView* pAttachmentView,
			const VkImageLayout attachmentLayout,
			const VkResolveModeFlags resolveFlags,
			const TextureView* pResolveAttachmentView,
			const VkImageLayout resolveAttachmentLayout,
			const VkAttachmentLoadOp attachmentLoadOp,
			const VkAttachmentStoreOp attachmentStoreOp,
			const VkClearValue attachmentClearValue);
		

		void BeginDynamicRendering(const unsigned int viewMask, const unsigned int layerCount, const int x, const int y, const unsigned int width, const unsigned height);
		void EndDynamicRendering();
		void ClearDynamicRenderingColorAttachments();
		void ClearDynamicRenderingDepthAttachment();
		void ClearDynamicRenderingStencilAttachment();
		void SetPipeline(const Pipeline* pPipeline);


		void CopyBufferBuffer(
			GraphicsBuffer* pSrcBuffer,GraphicsBuffer* pDstBuffer,
			const unsigned long long srcOffset,const unsigned long long dstOffset,const unsigned long long size);
		void CopyBufferTexture(
		const GraphicsBuffer* pBuffer,
		const Texture* pTexture,
		const VkImageLayout textureLayout,
		const unsigned long long bufferOffset,const unsigned int bufferRowLength,const unsigned int bufferImageHeight,
		const VkImageAspectFlags textureAspectMask,const unsigned char mipIndex,const unsigned char arrayCopyStartIndex,const unsigned int arrayCopyCount,
		const int textureX,const int textureY,const int textureZ,
		const unsigned int textureWidth,const unsigned int textureHeight,const unsigned int textureDepth);

		void SetVertexBuffers(const GraphicsBuffer** ppBuffers, const unsigned char bufferCount, const unsigned char firstBinding, const unsigned long long* pOffsets);
		void SetIndexBuffer(const GraphicsBuffer* pBuffer, const unsigned long long offset, const VkIndexType type);

		void SetViewport(const VkViewport* pViewports, const unsigned int viewportCount,const unsigned int firstViewport);
		void SetScissor(const VkRect2D* pRects,const unsigned int scissorCount,const unsigned int firstScissor);

		void SetViewports(const VkViewport* pViewports, const unsigned int count);
		void SetScissors(const VkRect2D* pRects, const unsigned int count);

		void SetDescriptorSets(const DescriptorSet** ppSets, const unsigned int setCount,const unsigned int firstSetIndex,const unsigned int* pDynamicOffsets,const unsigned int dynamicOffsetCount);

		void DrawIndexed(const unsigned int indexCount, const unsigned int instanceCount, const unsigned int firstIndex,const unsigned int vertexOffset, const unsigned int firstInstance);

	private:
		const VkDevice mLogicalDevice;
		const VkCommandPool mPool;
		const GraphicsDevice* mDevice;
		VkCommandBuffer mCmdBuffer;

		const Pipeline* mBoundPipeline;

		std::vector<VkRenderingAttachmentInfo> mDynamicRenderingColorAttachments;
		VkRenderingAttachmentInfo mDynamicRenderingDepthAttachment;
		VkRenderingAttachmentInfo mDynamicRenderingStencilAttachment;
		bool mHasDynamicRenderingDepthAttachment;
		bool mHasDynamicRenderingStencilAttachment;
	};
}
#include "CommandList.h"
#include <Runtime/Graphics/Device/GraphicsDevice.h>
#include <Runtime/Graphics/Command/CommandPool.h>
#include <Runtime/Graphics/Buffer/GraphicsBuffer.h>

namespace Oksijen
{
	CommandList::CommandList(const GraphicsDevice* pDevice, const CommandPool* pPool) 
		: mLogicalDevice(pDevice->GetLogicalDevice()),mPool(pPool->GetCommandPool()),mDevice(pDevice), mCmdBuffer(VK_NULL_HANDLE),
		mDynamicRenderingDepthAttachment(),mDynamicRenderingStencilAttachment(), mHasDynamicRenderingDepthAttachment(false), mHasDynamicRenderingStencilAttachment(false),
		mBoundPipeline(nullptr)
	{
		VkCommandBufferAllocateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		info.commandPool = mPool;
		info.commandBufferCount = 1;
		info.pNext = nullptr;

		DEV_ASSERT(vkAllocateCommandBuffers(mLogicalDevice, &info, &mCmdBuffer) == VK_SUCCESS, "CommandList", "Failed to allocate command buffers");

	}
	CommandList::~CommandList()
	{
		vkFreeCommandBuffers(mLogicalDevice, mPool, 1, &mCmdBuffer);
	}
	void CommandList::Begin()
	{
		DEV_ASSERT(vkResetCommandBuffer(mCmdBuffer, VkCommandPoolResetFlags()) == VK_SUCCESS, "CommandList", "Failed to reset the command list");

		VkCommandBufferBeginInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		info.flags = 0;
		info.pInheritanceInfo = nullptr;
		info.pNext = nullptr;

		DEV_ASSERT(vkBeginCommandBuffer(mCmdBuffer, &info) == VK_SUCCESS, "CommandList", "Failed to begin the command list");
	}
	void CommandList::End()
	{
		DEV_ASSERT(vkEndCommandBuffer(mCmdBuffer) == VK_SUCCESS, "CommandList", "Failed to end the command list");

		mBoundPipeline = nullptr;

		mHasDynamicRenderingDepthAttachment = false;
		mHasDynamicRenderingStencilAttachment = false;
		mDynamicRenderingColorAttachments.clear();
	}
	void CommandList::SetPipelineTextureBarrier(
		const Texture* pTexture,
		const VkImageLayout oldImageLayout, const VkImageLayout newImageLayout,
		const VkQueueFlags srcQueue, const VkQueueFlags dstQueue,
		const VkImageAspectFlags imageAspectMask, const unsigned char mipIndex, const unsigned char arrayIndex,
		const VkAccessFlags srcAccessMask, const VkAccessFlags dstAccessMask,
		const VkPipelineStageFlags srcPipelineStageMask, const VkPipelineStageFlags dstPipelineStageMask,
		const VkDependencyFlags dependencies)
	{
		VkImageMemoryBarrier barrierInfo = {};
		barrierInfo.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrierInfo.image = pTexture->GetImage();
		barrierInfo.oldLayout = oldImageLayout;
		barrierInfo.newLayout = newImageLayout;
		barrierInfo.srcQueueFamilyIndex = mDevice->GetQueueFamilyIndex(srcQueue);
		barrierInfo.dstQueueFamilyIndex = mDevice->GetQueueFamilyIndex(dstQueue);
		barrierInfo.subresourceRange.aspectMask = imageAspectMask;
		barrierInfo.subresourceRange.baseMipLevel = mipIndex;
		barrierInfo.subresourceRange.levelCount = pTexture->GetMipLevels();
		barrierInfo.subresourceRange.baseArrayLayer = arrayIndex;
		barrierInfo.subresourceRange.layerCount = pTexture->GetArrayLevels();
		barrierInfo.pNext = nullptr;

		vkCmdPipelineBarrier(mCmdBuffer, srcPipelineStageMask, dstPipelineStageMask, dependencies, 0, nullptr, 0, nullptr, 1, &barrierInfo);
	}
	void CommandList::SetPipelineTextureBarriers(
		const Texture** ppTextures,
		const VkImageLayout* pOldImageLayouts, const VkImageLayout* pNewImageLayouts,
		const VkQueueFlags* pSrcQueues, const VkQueueFlags* pDstQueues,
		const VkImageAspectFlags* pImageAspectMasks, const unsigned char* pMipIndexes, const unsigned char* pArrayIndexes,
		const VkAccessFlags* pSrcAccessMasks, const VkAccessFlags* pDstAccessMasks,
		const VkPipelineStageFlags srcPipelineStageMask, const VkPipelineStageFlags dstPipelineStageMask,
		const VkDependencyFlags dependencies,
		const unsigned int count)
	{
		DEV_ASSERT(count > 0, "CommandList", "SetPipelineTextureBarriers count parameter should >0");

		VkImageMemoryBarrier barriers[128];

		for (unsigned int i = 0; i < count; i++)
		{
			VkImageMemoryBarrier barrierInfo = {};
			barrierInfo.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrierInfo.image = ppTextures[i]->GetImage();
			barrierInfo.oldLayout = pOldImageLayouts[i];
			barrierInfo.newLayout = pNewImageLayouts[i];
			barrierInfo.srcQueueFamilyIndex = mDevice->GetQueueFamilyIndex(pSrcQueues[i]);
			barrierInfo.dstQueueFamilyIndex = mDevice->GetQueueFamilyIndex(pDstQueues[i]);
			barrierInfo.subresourceRange.aspectMask = pImageAspectMasks[i];
			barrierInfo.subresourceRange.baseMipLevel = pMipIndexes[i];
			barrierInfo.subresourceRange.levelCount = ppTextures[i]->GetMipLevels();
			barrierInfo.subresourceRange.baseArrayLayer = pArrayIndexes[i];
			barrierInfo.subresourceRange.layerCount = ppTextures[i]->GetArrayLevels();
			barrierInfo.pNext = nullptr;
		}

		vkCmdPipelineBarrier(mCmdBuffer, srcPipelineStageMask, dstPipelineStageMask, dependencies, 0, nullptr, 0, nullptr, 1, barriers);
	}
	void CommandList::SetPipelineMemoryBarrier(const VkAccessFlags srcAccessMask, const VkAccessFlags dstAccessMask,
		const VkPipelineStageFlags srcPipelineStageMask, const VkPipelineStageFlags dstPipelineStageMask,
		const VkDependencyFlags dependencies)
	{
		VkMemoryBarrier barrier = {};
		barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
		barrier.srcAccessMask = srcAccessMask;
		barrier.dstAccessMask = dstAccessMask;
		barrier.pNext = nullptr;

		vkCmdPipelineBarrier(mCmdBuffer, srcPipelineStageMask, dstPipelineStageMask, dependencies, 1, &barrier, 0, nullptr, 0, nullptr);
	}
	void CommandList::SetPipelineMemoryBarriers(
		const VkAccessFlags* pSrcAccessMasks, const VkAccessFlags* pDstAccessMasks,
		const VkPipelineStageFlags srcPipelineStageMask, const VkPipelineStageFlags dstPipelineStageMask,
		const VkDependencyFlags dependencies,
		const unsigned int count)
	{
		DEV_ASSERT(count > 0, "CommandList", "SetMemoryBarriers count parameter should be >0");

		VkMemoryBarrier barriers[32];
		for (unsigned int i = 0; i < count; i++)
		{
			VkMemoryBarrier barrier = {};
			barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
			barrier.srcAccessMask = pSrcAccessMasks[i];
			barrier.dstAccessMask = pDstAccessMasks[i];
			barrier.pNext = nullptr;

			barriers[i] = barrier;
		}

		vkCmdPipelineBarrier(mCmdBuffer, srcPipelineStageMask, dstPipelineStageMask, dependencies, count, barriers, 0, nullptr, 0, nullptr);
	}
	void CommandList::SetPipelineBufferBarrier(const GraphicsBuffer* pBuffer,
		const VkAccessFlags srcAccessMask, const VkAccessFlags dstAccessMask,
		const VkQueueFlags srcQueue, const VkQueueFlags dstQueue,
		const unsigned long long offset, const unsigned long long size,
		const VkPipelineStageFlags srcPipelineStageMask, const VkPipelineStageFlags dstPipelineStageMask,
		const VkDependencyFlags dependencies)
	{
		VkBufferMemoryBarrier barrier = {};
		barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
		barrier.buffer = pBuffer->GetBuffer();
		barrier.srcAccessMask = srcAccessMask;
		barrier.dstAccessMask = dstAccessMask;
		barrier.srcQueueFamilyIndex = mDevice->GetQueueFamilyIndex(srcQueue);
		barrier.dstQueueFamilyIndex = mDevice->GetQueueFamilyIndex(dstQueue);
		barrier.offset = offset;
		barrier.size = size;
		barrier.pNext = nullptr;

		vkCmdPipelineBarrier(mCmdBuffer, srcPipelineStageMask, dstPipelineStageMask, dependencies, 0, nullptr, 1, &barrier, 0, nullptr);
	}
	void CommandList::SetPipelineBufferBarriers(const GraphicsBuffer** ppBuffers,
		const VkAccessFlags* pSrcAccessMasks, const VkAccessFlags* pDstAccessMasks,
		const VkQueueFlags* pSrcQueues, const VkQueueFlags* pDstQueues,
		const unsigned long long* pOffsets, const unsigned long long* pSizes,
		const VkPipelineStageFlags srcPipelineStageMask, const VkPipelineStageFlags dstPipelineStageMask,
		const VkDependencyFlags dependencies,
		const unsigned int count)
	{
		DEV_ASSERT(count > 0, "CommandList", "SetBufferBarriers count parameter should be >0");

		VkBufferMemoryBarrier barriers[32];

		for (unsigned int i = 0; i < count; i++)
		{
			VkBufferMemoryBarrier barrier = {};
			barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
			barrier.buffer = ppBuffers[i]->GetBuffer();
			barrier.srcAccessMask = pSrcAccessMasks[i];
			barrier.dstAccessMask = pDstAccessMasks[i];
			barrier.srcQueueFamilyIndex = mDevice->GetQueueFamilyIndex(pSrcQueues[i]);
			barrier.dstQueueFamilyIndex = mDevice->GetQueueFamilyIndex(pDstQueues[i]);
			barrier.offset = pOffsets[i];
			barrier.size = pSizes[i];
			barrier.pNext = nullptr;

			barriers[i] = barrier;
		}

		vkCmdPipelineBarrier(mCmdBuffer, srcPipelineStageMask, dstPipelineStageMask, dependencies, 0, nullptr, count, barriers, 0, nullptr);
	}
	
	void CommandList::SetPipelineBarriers(
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
		const unsigned int memoryCount, const unsigned int bufferCount, const unsigned int textureCount)
	{
		//Collect memory barriers
		VkMemoryBarrier memoryBarriers[32];
		for (unsigned int i = 0; i < memoryCount; i++)
		{
			VkMemoryBarrier barrier = {};
			barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
			barrier.srcAccessMask = pMemorySrcAccessMask[i];
			barrier.dstAccessMask = pMemoryDstAccessMask[i];
			barrier.pNext = nullptr;

			memoryBarriers[i] = barrier;
		}

		//Collect buffer barriers
		VkBufferMemoryBarrier bufferBarriers[32];
		for (unsigned int i = 0; i < bufferCount; i++)
		{
			VkBufferMemoryBarrier barrier = {};
			barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
			barrier.buffer = ppBuffers[i]->GetBuffer();
			barrier.srcAccessMask = pBufferSrcAccessMasks[i];
			barrier.dstAccessMask = pBufferDstAccessMasks[i];
			barrier.srcQueueFamilyIndex = mDevice->GetQueueFamilyIndex(pBufferSrcQueues[i]);
			barrier.dstQueueFamilyIndex = mDevice->GetQueueFamilyIndex(pBufferDstQueues[i]);
			barrier.offset = pBufferOffsets[i];
			barrier.size = pBufferSizes[i];
			barrier.pNext = nullptr;

			bufferBarriers[i] = barrier;
		}

		//Collect image barriers
		VkImageMemoryBarrier imageBarriers[32];
		for (unsigned int i = 0; i < textureCount; i++)
		{
			VkImageMemoryBarrier barrier = {};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.image = ppTextures[i]->GetImage();
			barrier.oldLayout = pTextureOldImageLayouts[i];
			barrier.newLayout = pTextureNewImageLayouts[i];
			barrier.srcQueueFamilyIndex = mDevice->GetQueueFamilyIndex(pTextureSrcQueues[i]);
			barrier.dstQueueFamilyIndex = mDevice->GetQueueFamilyIndex(pTextureDstQueues[i]);
			barrier.subresourceRange.aspectMask = pTextureAspectMasks[i];
			barrier.subresourceRange.baseMipLevel = pTextureMipIndexes[i];
			barrier.subresourceRange.levelCount = ppTextures[i]->GetMipLevels();
			barrier.subresourceRange.baseArrayLayer = pTextureArrayIndexes[i];
			barrier.subresourceRange.layerCount = ppTextures[i]->GetArrayLevels();
			barrier.pNext = nullptr;

			imageBarriers[i] = barrier;
		}
	}

	void CommandList::AddDynamicRenderingColorAttachment(
		const TextureView* pAttachmentView,
		const VkImageLayout attachmentLayout,
		const VkResolveModeFlags resolveFlags,
		const TextureView* pResolveAttachmentView,
		const VkImageLayout resolveAttachmentLayout,
		const VkAttachmentLoadOp attachmentLoadOp,
		const VkAttachmentStoreOp attachmentStoreOp,
		const VkClearColorValue attachmentClearValue)
	{

		VkClearValue clearValue = {};
		clearValue.color = attachmentClearValue;

		VkRenderingAttachmentInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
		info.imageView = pAttachmentView->GetView();
		info.imageLayout = attachmentLayout;
		info.resolveMode = (VkResolveModeFlagBits)resolveFlags;
		info.resolveImageView = pResolveAttachmentView != nullptr ? pResolveAttachmentView->GetView() : nullptr;
		info.resolveImageLayout = resolveAttachmentLayout;
		info.storeOp = attachmentStoreOp;
		info.loadOp = attachmentLoadOp;
		info.clearValue = clearValue;
		info.pNext = nullptr;

		mDynamicRenderingColorAttachments.push_back(info);

	}

	void CommandList::SetDynamicRenderingDepthAttachment(
		const TextureView* pAttachmentView,
		const VkImageLayout attachmentLayout,
		const VkResolveModeFlags resolveFlags,
		const TextureView* pResolveAttachmentView,
		const VkImageLayout resolveAttachmentLayout,
		const VkAttachmentLoadOp attachmentLoadOp,
		const VkAttachmentStoreOp attachmentStoreOp,
		const VkClearValue attachmentClearValue)
	{
		VkRenderingAttachmentInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
		info.imageView = pAttachmentView->GetView();
		info.imageLayout = attachmentLayout;
		info.resolveMode = (VkResolveModeFlagBits)resolveFlags;
		info.resolveImageView = pResolveAttachmentView != nullptr ? pResolveAttachmentView->GetView() : nullptr;
		info.resolveImageLayout = resolveAttachmentLayout;
		info.storeOp = attachmentStoreOp;
		info.loadOp = attachmentLoadOp;
		info.clearValue = attachmentClearValue;
		info.pNext = nullptr;

		mDynamicRenderingDepthAttachment = info;
		mHasDynamicRenderingDepthAttachment = true;
	}

	void CommandList::SetDynamicRenderingStencilAttachment(
		const TextureView* pAttachmentView,
		const VkImageLayout attachmentLayout,
		const VkResolveModeFlags resolveFlags,
		const TextureView* pResolveAttachmentView,
		const VkImageLayout resolveAttachmentLayout,
		const VkAttachmentLoadOp attachmentLoadOp,
		const VkAttachmentStoreOp attachmentStoreOp,
		const VkClearValue attachmentClearValue)
	{
		VkRenderingAttachmentInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
		info.imageView = pAttachmentView->GetView();
		info.imageLayout = attachmentLayout;
		info.resolveMode = (VkResolveModeFlagBits)resolveFlags;
		info.resolveImageView = pResolveAttachmentView != nullptr ? pResolveAttachmentView->GetView() : nullptr;
		info.resolveImageLayout = resolveAttachmentLayout;
		info.storeOp = attachmentStoreOp;
		info.loadOp = attachmentLoadOp;
		info.clearValue = attachmentClearValue;
		info.pNext = nullptr;

		mDynamicRenderingStencilAttachment = info;
		mHasDynamicRenderingStencilAttachment = false;
	}

	

	
	void CommandList::BeginDynamicRendering(const unsigned int viewMask, const unsigned int layerCount, const int x, const int y, const unsigned int width, const unsigned height)
	{
		VkRenderingInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
		info.colorAttachmentCount = mDynamicRenderingColorAttachments.size();
		info.pColorAttachments = mDynamicRenderingColorAttachments.data();
		info.pDepthAttachment = mHasDynamicRenderingDepthAttachment ? &mDynamicRenderingDepthAttachment : nullptr;
		info.pStencilAttachment = mHasDynamicRenderingStencilAttachment ? &mDynamicRenderingStencilAttachment : nullptr;
		info.viewMask = viewMask;
		info.layerCount = layerCount;
		info.renderArea.offset = { x,y };
		info.renderArea.extent = { width,height };
		info.pNext = nullptr;

		vkCmdBeginRendering(mCmdBuffer, &info);
	}

	void CommandList::EndDynamicRendering()
	{
		vkCmdEndRendering(mCmdBuffer);
	}
	void CommandList::ClearDynamicRenderingColorAttachments()
	{
		mDynamicRenderingColorAttachments.clear();
	}
	void CommandList::ClearDynamicRenderingDepthAttachment()
	{
		mHasDynamicRenderingDepthAttachment = false;
	}
	void CommandList::ClearDynamicRenderingStencilAttachment()
	{
		mHasDynamicRenderingStencilAttachment = false;
	}
	void CommandList::SetPipeline(const Pipeline* pPipeline)
	{
		vkCmdBindPipeline(mCmdBuffer, pPipeline->GetBindPoint(), pPipeline->GetPipeline());
		mBoundPipeline = pPipeline;
	}
	void CommandList::DispatchCompute(const unsigned x, const unsigned y, const unsigned z)
	{
		vkCmdDispatch(mCmdBuffer, x, y, z);
	}
	void CommandList::CopyBufferBuffer(
		GraphicsBuffer* pSrcBuffer, GraphicsBuffer* pDstBuffer,
		const unsigned long long srcOffset, const unsigned long long dstOffset,const unsigned long long size)
	{
		VkBufferCopy info = {};
		info.srcOffset = srcOffset;
		info.dstOffset = dstOffset;
		info.size = size;
		vkCmdCopyBuffer(mCmdBuffer, pSrcBuffer->GetBuffer(), pDstBuffer->GetBuffer(),1,&info);
	}
	void CommandList::CopyBufferTexture(
		const GraphicsBuffer* pBuffer,
		const Texture* pTexture,
		const VkImageLayout textureLayout,
		const unsigned long long bufferOffset, const unsigned int bufferRowLength, const unsigned int bufferImageHeight,
		const VkImageAspectFlags textureAspectMask, const unsigned char mipIndex, const unsigned char arrayCopyStartIndex, const unsigned int arrayCopyCount,
		const int textureX, const int textureY, const int textureZ,
		const unsigned int textureWidth, const unsigned int textureHeight, const unsigned int textureDepth)
	{
		VkBufferImageCopy copy = {};
		copy.bufferOffset = bufferOffset;
		copy.bufferRowLength = bufferRowLength;
		copy.bufferImageHeight = bufferImageHeight;
		copy.imageSubresource.aspectMask = textureAspectMask;
		copy.imageSubresource.mipLevel = mipIndex;
		copy.imageSubresource.baseArrayLayer = arrayCopyStartIndex;
		copy.imageSubresource.layerCount = arrayCopyCount;
		copy.imageOffset = { textureX,textureY,textureZ };
		copy.imageExtent = { textureWidth,textureHeight,textureDepth };

		vkCmdCopyBufferToImage(mCmdBuffer, pBuffer->GetBuffer(), pTexture->GetImage(), textureLayout, 1, &copy);
	}
	void CommandList::SetVertexBuffers(const GraphicsBuffer** ppBuffers, const unsigned char bufferCount, const unsigned char firstBinding, const unsigned long long* pOffsets)
	{
		VkBuffer buffers[32];
		for (unsigned int i = 0; i < bufferCount; i++)
		{
			buffers[i] = ppBuffers[i]->GetBuffer();
		}
		vkCmdBindVertexBuffers(mCmdBuffer, firstBinding, bufferCount,buffers,pOffsets);
	}
	void CommandList::SetIndexBuffer(const GraphicsBuffer* pBuffer, const unsigned long long offset, const VkIndexType type)
	{
		vkCmdBindIndexBuffer(mCmdBuffer, pBuffer->GetBuffer(), offset, type);
	}
	void CommandList::PlaceViewport(const VkViewport* pViewports, const unsigned int viewportCount, const unsigned int firstViewport)
	{
		vkCmdSetViewport(mCmdBuffer, firstViewport, viewportCount, pViewports);
	}
	void CommandList::PlaceScissor(const VkRect2D* pRects, const unsigned int scissorCount, const unsigned int firstScissor)
	{
		vkCmdSetScissor(mCmdBuffer, firstScissor, scissorCount, pRects);
	}
	void CommandList::SetViewports(const VkViewport* pViewports, const unsigned int count)
	{
		vkCmdSetViewportWithCount(mCmdBuffer, count, pViewports);
	}
	void CommandList::SetScissors(const VkRect2D* pRects, const unsigned int count)
	{
		vkCmdSetScissorWithCount(mCmdBuffer, count, pRects);
	}
	void CommandList::SetDescriptorSets(const DescriptorSet** ppSets, const unsigned int setCount, const unsigned int firstSetIndex, const unsigned int* pDynamicOffsets, const unsigned int dynamicOffsetCount)
	{
		//Collect sets
		VkDescriptorSet sets[32];
		for (unsigned int i = 0; i < setCount; i++)
		{
			sets[i] = ppSets[i]->GetSet();
		}

		vkCmdBindDescriptorSets(mCmdBuffer,mBoundPipeline->GetBindPoint(),mBoundPipeline->GetLayout(),firstSetIndex,setCount,sets,dynamicOffsetCount,pDynamicOffsets);
	}
	void CommandList::DrawIndexed(const unsigned int indexCount, const unsigned int instanceCount, const unsigned int firstIndex, const unsigned int firstVertex, const unsigned int firstInstance)
	{
		vkCmdDrawIndexed(mCmdBuffer,indexCount,instanceCount,firstIndex, firstVertex,firstInstance);
	}
}
#include "CommandList.h"
#include <Runtime/Graphics/Device/GraphicsDevice.h>
#include <Runtime/Graphics/Command/CommandPool.h>

namespace Oksijen
{
	CommandList::CommandList(const GraphicsDevice* pDevice, const CommandPool* pPool) : mLogicalDevice(pDevice->GetLogicalDevice()),mPool(pPool->GetCommandPool()),mDevice(pDevice), mCmdBuffer(VK_NULL_HANDLE)
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
	}
	void CommandList::SetPipelineTextureBarrier(const Texture* pTexture, const VkImageLayout oldImageLayout, const VkImageLayout newImageLayout, const VkQueueFlags srcQueue, const VkQueueFlags dstQueue, const VkImageAspectFlags imageAspectMask, const unsigned char mipIndex, const unsigned char arrayIndex, const VkAccessFlags srcAccessMask, const VkAccessFlags dstAccessMask, const VkPipelineStageFlags srcPipelineStageMask, const VkPipelineStageFlags dstPipelineStageMask)
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

		vkCmdPipelineBarrier(mCmdBuffer, srcPipelineStageMask, dstPipelineStageMask, 0, 0, nullptr, 0, nullptr, 1, &barrierInfo);
	}
	void CommandList::BeginDynamicRendering(
		const VkRenderingAttachmentInfo* pColorAttachments, const unsigned char colorAttachmentCount,
		const VkRenderingAttachmentInfo* pDepthAttachment,
		const VkRenderingAttachmentInfo* pStencilAttachment,
		const int x, const int y,const unsigned int width, const unsigned height)
	{
		VkRenderingInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
		info.colorAttachmentCount = colorAttachmentCount;
		info.pColorAttachments = pColorAttachments;
		info.pDepthAttachment = pDepthAttachment;
		info.pStencilAttachment = pStencilAttachment;
		info.viewMask = 0;
		info.renderArea = {x,y,width,height};
		info.layerCount = 1;
		info.pNext = nullptr;
		vkCmdBeginRendering(mCmdBuffer,&info);
	}
	void CommandList::EndDynamicRendering()
	{
		vkCmdEndRendering(mCmdBuffer);
	}
}
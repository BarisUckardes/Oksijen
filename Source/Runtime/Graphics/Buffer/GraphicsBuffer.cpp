#include "GraphicsBuffer.h"
#include <Runtime/Graphics/Device/GraphicsDevice.h>
#include <Runtime/Graphics/Memory/GraphicsMemory.h>
namespace Oksijen
{
	GraphicsBuffer::GraphicsBuffer(const GraphicsDevice* pDevice, GraphicsMemory* pMemory, const VkBufferUsageFlags usages, const VkSharingMode sharingMode, const unsigned long long bufferSize) :
		mLogicalDevice(pDevice->GetLogicalDevice()),mMemory(pMemory),mSize(bufferSize), mMemoryOffset(uint64_max), mMemoryAlignedOffset(uint64_max)
	{
		//Create buffer
		VkBufferCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		info.usage = usages;
		info.size = bufferSize;
		info.sharingMode = sharingMode;
		info.flags = VkBufferCreateFlags();
		info.pNext = nullptr;

		DEV_ASSERT(vkCreateBuffer(mLogicalDevice, &info, nullptr, &mBuffer) == VK_SUCCESS, "GraphicsBuffer", "Failed to create the buffer");

		//Get memory requirements
		VkMemoryRequirements requirements = {};
		vkGetBufferMemoryRequirements(mLogicalDevice, mBuffer, &requirements);

		//Get buffer memory
		const unsigned long long memoryOffset = pMemory->Allocate(requirements.size + requirements.alignment);
		const unsigned long long alignedMemoryOffset = memoryOffset + (memoryOffset % requirements.alignment == 0 ? 0 : (requirements.alignment - (memoryOffset % requirements.alignment)));

		//Bind memory
		DEV_ASSERT(vkBindBufferMemory(mLogicalDevice, mBuffer, mMemory->GetMemory(), alignedMemoryOffset) == VK_SUCCESS, "VulkanBuffer", "Failed to bind memory");

		mMemoryOffset = memoryOffset;
		mMemoryAlignedOffset = alignedMemoryOffset;
	}
	GraphicsBuffer::~GraphicsBuffer()
	{
		//Free
		mMemory->Free(mMemoryOffset);

		//Destroy
		vkDestroyBuffer(mLogicalDevice, mBuffer, nullptr);
	}
}
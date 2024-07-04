#include "GraphicsMemory.h"
#include <Runtime/Core/Core.h>
#include <Runtime/Graphics/Device/GraphicsDevice.h>

namespace Oksijen
{
	unsigned int GetMemoryTypeIndex(const VkPhysicalDeviceMemoryProperties& properties, const VkMemoryPropertyFlags flags)
	{
		for (unsigned int typeIndex = 0; typeIndex < properties.memoryTypeCount; typeIndex++)
		{
			const VkMemoryType memoryType = properties.memoryTypes[typeIndex];
			if (memoryType.propertyFlags & flags)
				return typeIndex;
		}

		DEV_ASSERT(false, "VulkanMemory", "Failed to get memory heap index");
		return 0;
	}

	GraphicsMemory::GraphicsMemory(const GraphicsDevice* pDevice, const VkMemoryPropertyFlagBits flags, const unsigned long long size, const VkPhysicalDevice physicalDevice)
		: mFlags(flags),mSize(size),mOccupiedSize(0),mLogicalDevice(pDevice->GetLogicalDevice())
	{

		//Get physical device memory type indexes
		VkPhysicalDeviceMemoryProperties memoryProperties = {};
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

		//Allocate vk memory
		VkMemoryAllocateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		info.allocationSize = size;
		info.memoryTypeIndex = GetMemoryTypeIndex(memoryProperties, flags);
		info.pNext = nullptr;

		DEV_ASSERT(vkAllocateMemory(mLogicalDevice, &info, nullptr, &mMemory) == VK_SUCCESS, "GraphicsMemory", "Failed to allocate memory");
		
		//Suballocate first block
		SubAllocationBlock initialBlock = {};
		initialBlock.bOwned = false;
		initialBlock.SizeInBytes = size;
		mBlocks.push_back(initialBlock);

		DEV_LOG("GraphicsMemory", "Allocated %.4fMB", size / 1000000.0f);
	}
	GraphicsMemory::~GraphicsMemory()
	{
		vkFreeMemory(mLogicalDevice, mMemory, nullptr);
	}
	unsigned long long GraphicsMemory::Allocate(const unsigned long long size)
	{
		//Check enough space is left
		const unsigned long long sizeLeft = mSize - mOccupiedSize;
		if (sizeLeft < size)
			return uint64_max;

		//Look for an available space
		unsigned long long offset = 0;
		for (unsigned int i = 0; i < mBlocks.size(); i++)
		{
			SubAllocationBlock& block = mBlocks[i];
			if (block.bOwned)
			{
				offset += block.SizeInBytes;
				continue;
			}

			if (block.SizeInBytes < size)
			{
				offset += block.SizeInBytes;
				continue;
			}

			const unsigned long long sizeLeft = block.SizeInBytes - size;
			block.SizeInBytes = sizeLeft;

			SubAllocationBlock newBlock = {};
			newBlock.bOwned = true;
			newBlock.SizeInBytes = size;
			mBlocks.insert(mBlocks.begin() + i, newBlock);
			mOccupiedSize += size;
			return offset;
		}

		return uint64_max;
	}
	void GraphicsMemory::Free(const unsigned long long offset)
	{
		return;
		//Find and free owned memory
		unsigned long long currentOffset = 0;
		for (unsigned int i = 0; i < mBlocks.size(); i++)
		{
			SubAllocationBlock& block = mBlocks[i];
			if (currentOffset != offset)
			{
				currentOffset += block.SizeInBytes;
				continue;
			}

			block.bOwned = false;
			mOccupiedSize -= block.SizeInBytes;
			break;
		}

		//Compact
		for (unsigned int i = 0; i < mBlocks.size(); i++)
		{
			SubAllocationBlock& block = mBlocks[i];
			CompactReport report = CreateCompactReport(i);
			if (report.Min == report.Max)
				continue;

			Compact(report);

			if (report.Min < i)
			{
				const unsigned int diff = i - report.Min;
				i -= diff;
			}
		}
	}
	Oksijen::GraphicsMemory::CompactReport GraphicsMemory::CreateCompactReport(const unsigned int index)
	{
		//Check min
		unsigned int min = index;
		for (unsigned int i = index; i > 0; i--)
		{
			SubAllocationBlock& block = mBlocks[i];
			if (block.bOwned)
				break;

			min--;
		}

		//Check max
		unsigned int max = index;
		for (unsigned int i = index; i < mBlocks.size(); i++)
		{
			SubAllocationBlock& block = mBlocks[i];
			if (block.bOwned)
				break;

			max++;
		}

		return { min,max };
	}
	void GraphicsMemory::Compact(const CompactReport& report)
	{
		//Calculate size
		unsigned long long size = 0;
		for (unsigned int i = report.Min; i < report.Max; i++)
		{
			SubAllocationBlock& block = mBlocks[i];
			size += block.SizeInBytes;
		}

		//Remove nodes
		const unsigned int removeCount = report.Max - report.Min;
		for (unsigned int i = 0; i < removeCount; i++)
			mBlocks.erase(mBlocks.begin() + report.Min);

		//Add single node
		SubAllocationBlock block = {};
		block.SizeInBytes = size;
		block.bOwned = false;
		mBlocks.insert(mBlocks.begin() + report.Min, block);
	}
}
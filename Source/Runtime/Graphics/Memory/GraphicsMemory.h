#pragma once
#include <vulkan/vulkan.h>
#include <vector>

namespace Oksijen
{
	class GraphicsDevice;
	class RUNTIME_API GraphicsMemory final
	{
	private:
		struct SubAllocationBlock
		{
			bool bOwned;
			unsigned long long SizeInBytes;
		};
		struct CompactReport
		{
			unsigned int Min;
			unsigned int Max;
		};
	public:
		GraphicsMemory(const GraphicsDevice* pDevice,const VkMemoryPropertyFlagBits flags,const unsigned long long size,const VkPhysicalDevice physicalDevice);
		~GraphicsMemory();

		FORCEINLINE VkMemoryPropertyFlagBits GetFlags() const noexcept { return mFlags; }
		FORCEINLINE VkDeviceMemory GetMemory() const noexcept { return mMemory; }
		FORCEINLINE unsigned long long GetSize() const noexcept { return mSize; }
		FORCEINLINE unsigned long long GetOccupiedSize() const noexcept { return mOccupiedSize; }

		unsigned long long Allocate(const unsigned long long size);
		void Free(const unsigned long long offset);
	private:
		CompactReport CreateCompactReport(const unsigned int index);
		void Compact(const CompactReport& report);
	private:
		const VkDevice mLogicalDevice;
		const VkMemoryPropertyFlagBits mFlags;
		const unsigned long long mSize;
		VkDeviceMemory mMemory;
		unsigned long long mOccupiedSize;
		std::vector<SubAllocationBlock> mBlocks;
	};
}
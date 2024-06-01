#pragma once
#include <Runtime/Core/Core.h>
#include <vulkan/vulkan.h>

namespace Oksijen
{
	class GraphicsDevice;
	class GraphicsMemory;
	class RUNTIME_API GraphicsBuffer final
	{
	public:
		GraphicsBuffer(const GraphicsDevice* pDevice,GraphicsMemory* pMemory,const VkBufferUsageFlags usages,const VkSharingMode sharingMode,const unsigned long long bufferSize);
		~GraphicsBuffer();

		FORCEINLINE GraphicsMemory* GetMemory() const noexcept { return mMemory; }
		FORCEINLINE unsigned long long GetSize() const noexcept { return mSize; }
		FORCEINLINE VkBuffer GetBuffer() const noexcept { return mBuffer; }
		FORCEINLINE unsigned long long GetMemoryOffset() const noexcept { return mMemoryOffset; }
		FORCEINLINE unsigned long long GetMemoryAlignedOffset() const noexcept { return mMemoryAlignedOffset; }

	private:
		const VkDevice mLogicalDevice;
		const unsigned long long mSize;
		GraphicsMemory* mMemory;
		VkBuffer mBuffer;
		unsigned long long mMemoryOffset;
		unsigned long long mMemoryAlignedOffset;

	};
}
#pragma once
#include <Runtime/Core/Core.h>
#include <vulkan/vulkan.h>

namespace Oksijen
{
	class GraphicsDevice;
	class RUNTIME_API Semaphore final
	{
	public:
		Semaphore(const GraphicsDevice* pDevice,const bool bSignalled);
		~Semaphore();

		FORCEINLINE VkSemaphore GetSemaphore() const noexcept { return mSemaphore; }
	private:
		const VkDevice mLogicalDevice;
		VkSemaphore mSemaphore;
	};
}
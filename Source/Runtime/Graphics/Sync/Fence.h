#pragma once
#include <Runtime/Core/Core.h>
#include <vulkan/vulkan.h>

namespace Oksijen
{
	class GraphicsDevice;
	class RUNTIME_API Fence final
	{
	public:
		Fence(const GraphicsDevice* pDevice,const bool bSignalled);
		~Fence();

		FORCEINLINE VkFence GetFence() const noexcept { return mFence; }

		void Wait(const unsigned long long waitAmount = uint64_max);
		void Reset();
	private:
		const VkDevice mLogicalDevice;
		VkFence mFence;
	};
}
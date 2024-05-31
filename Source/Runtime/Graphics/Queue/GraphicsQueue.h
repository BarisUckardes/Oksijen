#pragma once
#include <Runtime/Core/Core.h>
#include <vulkan/vulkan.h>

namespace Oksijen
{
	class GraphicsDevice;
	class RUNTIME_API GraphicsQueue final
	{
	public:
		GraphicsQueue(const VkQueue queue,const VkQueueFlags flags,const unsigned int familyIndex,GraphicsDevice* pDevice);
		~GraphicsQueue();

		FORCEINLINE VkQueue GetQueue() const noexcept { return mQueue; }
		FORCEINLINE unsigned int GetFamilyIndex() const noexcept { return mFamilyIndex; }
		FORCEINLINE VkQueueFlags GetFlags() const noexcept { return mFlags; }
	private:
	private:
		const VkQueue mQueue;
		const unsigned int mFamilyIndex;
		const VkQueueFlags mFlags;
		GraphicsDevice* mOwnerDevice;
	};
}
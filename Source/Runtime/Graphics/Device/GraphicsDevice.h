#pragma once
#include <vulkan/vulkan.h>
#include <Runtime/Graphics/Queue/GraphicsQueue.h>
#include <Runtime/Graphics/Swapchain/Swapchain.h>
#include <Runtime/Graphics/Texture/Texture.h>
#include <Runtime/Graphics/Sync/Fence.h>
#include <Runtime/Graphics/Sync/Semaphore.h>
#include <Runtime/Graphics/Memory/GraphicsMemory.h>
#include <Runtime/Graphics/Command/CommandPool.h>
#include <Runtime/Graphics/Command/CommandList.h>
#include <vector>
#include <string>

namespace Oksijen
{
	struct QueueFamilyRequestInfo
	{
		VkQueueFlags Flags;
		unsigned int Count;
	};

	class GraphicsAdapter;
	class RUNTIME_API GraphicsDevice final
	{
		friend class GraphicsAdapter;
		friend class GraphicsQueue;
	private:
		struct DeviceQueueFamily
		{
		public:
			DeviceQueueFamily(const VkQueueFlags flags,const unsigned char familyIndex,const unsigned char requestedCount) : Flags(flags), FamilyIndex(familyIndex), RequestedCount(requestedCount)
			{

			}
			DeviceQueueFamily() : FamilyIndex(255), RequestedCount(0),Flags(VkQueueFlags())
			{

			}

			FORCEINLINE VkQueueFlags GetFlags() const noexcept { return Flags; }
			FORCEINLINE unsigned char GetFamilyIndex() const noexcept { return FamilyIndex; }
			FORCEINLINE unsigned char GetRequestedCount() const noexcept { return RequestedCount; }

			VkQueue OwnQueue()
			{
				if (FreeQueues.size() == 0)
					return VK_NULL_HANDLE;

				VkQueue queue = FreeQueues[0];
				FreeQueues.erase(FreeQueues.begin());
				return queue;
			}

			void ReturnQueue(VkQueue queue)
			{
				FreeQueues.push_back(queue);
			}

		private:
			const VkQueueFlags Flags;
			const unsigned char RequestedCount;
			unsigned char FamilyIndex;
			std::vector<VkQueue> FreeQueues;
		};
	public:
		~GraphicsDevice();

		FORCEINLINE GraphicsAdapter* GetOwnerAdapter() const noexcept { return mOwnerAdapter; }
		FORCEINLINE VkDevice GetLogicalDevice() const noexcept { return mLogicalDevice; }

		unsigned char GetQueueFamilyIndex(const VkQueueFlags flags) const noexcept;
		GraphicsQueue* RentQueue(const VkQueueFlags flags);
		Swapchain* CreateSwapchain(const Surface* pSurface, GraphicsQueue* pQueue, const unsigned int bufferCount, const unsigned int width, const unsigned int height, const VkPresentModeKHR presentMode, const VkFormat format, const VkColorSpaceKHR formatColorspace, const VkImageUsageFlags imageUsageFlags);
		GraphicsMemory* AllocateMemory(const VkMemoryPropertyFlagBits flags,const unsigned long long size);
		Texture* CreateTexture(GraphicsMemory* pMemory, const VkImageType type, const VkFormat format, const unsigned int width, const unsigned int height,const unsigned int depth, const unsigned char mipLevels, const unsigned char arrayLevels, const VkSampleCountFlagBits samples, const VkImageTiling tiling, const VkImageUsageFlags usageFlags, const VkSharingMode sharingMode, const VkImageLayout imageLayout);
		Fence* CreateFence(bool bSignalled);
		Semaphore* CreateSemaphore(bool bSignalled);
		CommandPool* CreateCommandPool(const VkQueueFlags flags);
		CommandList* AllocateCommandList(const CommandPool* pPool);

		void SubmitCommandLists(const GraphicsQueue* pQueue,const CommandList** ppCmdLists, const unsigned int cmdListCount,const VkPipelineStageFlags* pWaitDstPipelineStageFlags,const Fence* pWaitFence,const Semaphore** ppWaitSemaphores,const unsigned int waitSemaphoreCount,const Semaphore** ppSignalSemaphores,const unsigned int signalSemaphoreCount);
	private:
		GraphicsDevice(const std::vector<std::string>& extensions,const std::vector<std::string>& layers,const std::vector<QueueFamilyRequestInfo>& queueFamilyRequests,const VkPhysicalDeviceFeatures* pFeatures,const void* pDeviceNext,GraphicsAdapter* pAdapter);

		void SignalQueueReturned(const VkQueue queue, const unsigned int familyIndex);
	private:
		const VkPhysicalDevice mPhysicalDevice;
		GraphicsAdapter* mOwnerAdapter;
		VkDevice mLogicalDevice;
		std::vector<DeviceQueueFamily> mQueueFamilies;
	};
}
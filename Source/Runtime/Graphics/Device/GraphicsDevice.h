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
#include <Runtime/Graphics/Buffer/GraphicsBuffer.h>
#include <Runtime/Graphics/Shader/Shader.h>
#include <Runtime/Graphics/Pipeline/GraphicsPipeline.h>
#include <Runtime/Graphics/Pipeline/ComputePipeline.h>
#include <Runtime/Graphics/RenderPass/RenderPass.h>
#include <Runtime/Graphics/Texture/TextureView.h>
#include <Runtime/Graphics/Sampler/Sampler.h>
#include <Runtime/Graphics/Descriptor/DescriptorPool.h>
#include <Runtime/Graphics/Descriptor/DescriptorSetLayout.h>
#include <Runtime/Graphics/Descriptor/DescriptorSet.h>
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
		Swapchain* CreateSwapchain(const Surface* pSurface, GraphicsQueue* pQueue, const unsigned int bufferCount, const unsigned int width, const unsigned int height,const unsigned int arrayLevels, const VkPresentModeKHR presentMode, const VkFormat format, const VkColorSpaceKHR formatColorspace, const VkImageUsageFlags imageUsageFlags);
		GraphicsMemory* AllocateMemory(const VkMemoryPropertyFlagBits flags,const unsigned long long size);
		Texture* CreateTexture(
			GraphicsMemory* pMemory,
			const VkImageType type,
			const VkFormat format,
			const unsigned int width, const unsigned int height,const unsigned int depth,
			const unsigned char mipLevels, const unsigned char arrayLevels,
			const VkSampleCountFlagBits samples,
			const VkImageTiling tiling,
			const VkImageUsageFlags usageFlags,
			const VkSharingMode sharingMode,
			const VkImageLayout imageLayout);
		Texture* CreateTexture(
			GraphicsMemory* pMemory,
			const VkImageType type,
			const VkFormat format,
			const unsigned int width, const unsigned int height, const unsigned int depth,
			const unsigned char mipLevels, const unsigned char arrayLevels,
			const VkSampleCountFlagBits samples,
			const VkImageTiling tiling,
			const VkImageUsageFlags usageFlags,
			const VkSharingMode sharingMode,
			const VkImageLayout imageLayout,
			const VkImageCreateFlags flags);
		TextureView* CreateTextureView(const Texture* pTexture, const unsigned int mipIndex, const unsigned int arrayIndex, const VkImageViewType viewType, const VkFormat format, const VkComponentMapping mapping, const VkImageAspectFlags aspectMask);
		Fence* CreateFence(bool bSignalled);
		Semaphore* CreateSemaphore(bool bSignalled);
		CommandPool* CreateCommandPool(const VkQueueFlags flags);
		CommandList* AllocateCommandList(const CommandPool* pPool);
		Shader* CreateShader(const std::string& entryPoint, const VkShaderStageFlags stage, const unsigned char* pBytes, const unsigned int byteCount);
		GraphicsBuffer* CreateBuffer(GraphicsMemory* pMemory, const VkBufferUsageFlags usages, const VkSharingMode sharingMode, const unsigned long long bufferSize);
		GraphicsPipeline* CreateGraphicsPipeline(
			const VkPipelineVertexInputStateCreateInfo& vertexInputState,
			const VkPipelineInputAssemblyStateCreateInfo& inputAssemblyState,
			const VkPipelineTessellationStateCreateInfo* pTesellationState,
			const VkPipelineViewportStateCreateInfo* pViewportState,
			const VkPipelineRasterizationStateCreateInfo& rasterizerState,
			const VkPipelineMultisampleStateCreateInfo& multisampleState,
			const VkPipelineDepthStencilStateCreateInfo& depthStencilState,
			const VkPipelineColorBlendStateCreateInfo& blendingState,
			const VkPipelineDynamicStateCreateInfo* pDynamicState,
			const Shader** ppShaders, const unsigned int shaderCount,
			const DescriptorSetLayout** ppDescriptorLayouts, const unsigned int descriptorLayoutCount,
			const RenderPass* pRenderPass,
			const Pipeline* pBasePipeline, const unsigned int basePipelineIndex,
			const void* pNext);
		ComputePipeline* CreateComputePipeline(
			const VkPipelineCreateFlags flags,
			const Shader* pShader,
			const DescriptorSetLayout** ppSetLayouts, const unsigned int setLayoutCount,
			const Pipeline* pBasePipeline, const unsigned int basePipelineIndex);
		Sampler* CreateSampler(
			const VkFilter magFilter, const VkFilter minFilter,
			const VkSamplerMipmapMode mipmapMode, const VkSamplerAddressMode u, const VkSamplerAddressMode v, const VkSamplerAddressMode w,
			const float mipLodBias,
			const VkBool32 bAnisotropyEnabled, const float maxAnisotropy,
			const VkBool32 bCompareEnabled, const VkCompareOp compareOp,
			const float minLod, const float maxLod,
			const VkBorderColor borderColor,
			const VkBool32 unnormalizedCoordinates);
		DescriptorPool* CreateDescriptorPool(
			const VkDescriptorPoolCreateFlags flags,
			const unsigned int maxSets,
			const VkDescriptorType* pDescriptorTypes, const unsigned int* pDescriptorCounts,
			const unsigned int descriptorTypeCount);
		DescriptorSetLayout* CreateDescriptorSetLayout(
			const unsigned int* pBindings,
			const VkDescriptorType* pDescriptorTypes,
			const unsigned int* pDescriptorCounts,
			const VkShaderStageFlags* pStageFlags,
			const unsigned char bindingCount);
		DescriptorSet* AllocateDescriptorSet(const DescriptorPool* pPool, const DescriptorSetLayout* pLayout);

		void UpdateHostBuffer(GraphicsBuffer* pBuffer, const unsigned char* pData, const unsigned long long dataSize, const unsigned long long dstBufferOffset);

		void UpdateDescriptorSetTextureSampler(
			const DescriptorSet* pSet,
			const unsigned int dstBinding,
			const unsigned int dstArrayElement,
			const VkDescriptorType type,
			const TextureView* pTextureView,
			const Sampler* pSampler,
			const VkImageLayout textureLayout);
		void UpdateDescriptorSetBuffer(
		const DescriptorSet* pSet,
		const unsigned int dstBinding,
		const unsigned int dstArrayElement,
		const VkDescriptorType type,
		const GraphicsBuffer* pBuffer,
		const unsigned long long offset,const unsigned long long range);

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
#include "GraphicsDevice.h"
#include <Runtime/Core/Core.h>
#include <Runtime/Graphics/Adapter/GraphicsAdapter.h>

namespace Oksijen
{
	unsigned char GraphicsDevice::GetQueueFamilyIndex(const VkQueueFlags flags) const noexcept
	{
		for (const DeviceQueueFamily& family : mQueueFamilies)
		{
			if (family.GetFlags() & flags)
				return family.GetFamilyIndex();
		}

		return 255;
	}
	GraphicsQueue* GraphicsDevice::RentQueue(const VkQueueFlags flags)
	{
		for (DeviceQueueFamily& family : mQueueFamilies)
		{
			if (family.GetFlags() & flags)
			{
				VkQueue queue = family.OwnQueue();
				if (queue == VK_NULL_HANDLE)
					return nullptr;

				return new GraphicsQueue(queue, flags, family.GetFamilyIndex(), this);
			}
		}

		return nullptr;
	}
	Swapchain* GraphicsDevice::CreateSwapchain(const Surface* pSurface, GraphicsQueue* pQueue, const unsigned int bufferCount, const unsigned int width, const unsigned int height,const unsigned int arrayLevels, const VkPresentModeKHR presentMode, const VkFormat format, const VkColorSpaceKHR formatColorspace, const VkImageUsageFlags imageUsageFlags)
	{
		return new Swapchain(pSurface,pQueue,this,bufferCount,width,height,arrayLevels,presentMode,format,formatColorspace,imageUsageFlags);
	}
	GraphicsMemory* GraphicsDevice::AllocateMemory(const VkMemoryPropertyFlagBits flags, const unsigned long long size)
	{
		return new GraphicsMemory(this,flags,size,mOwnerAdapter->GetPhysicalDevice());
	}
	Texture* GraphicsDevice::CreateTexture(GraphicsMemory* pMemory, const VkImageType type, const VkFormat format, const unsigned int width, const unsigned int height,const unsigned int depth, const unsigned char mipLevels, const unsigned char arrayLevels, const VkSampleCountFlagBits samples, const VkImageTiling tiling, const VkImageUsageFlags usageFlags, const VkSharingMode sharingMode, const VkImageLayout imageLayout)
	{
		return new Texture(this,pMemory,type,format,width,height,depth,mipLevels,arrayLevels,samples,tiling,usageFlags,sharingMode,imageLayout);
	}
	Texture* GraphicsDevice::CreateTexture(GraphicsMemory* pMemory, const VkImageType type, const VkFormat format, const unsigned int width, const unsigned int height, const unsigned int depth, const unsigned char mipLevels, const unsigned char arrayLevels, const VkSampleCountFlagBits samples, const VkImageTiling tiling, const VkImageUsageFlags usageFlags, const VkSharingMode sharingMode, const VkImageLayout imageLayout,const VkImageCreateFlags flags)
	{
		return new Texture(this, pMemory, type, format, width, height, depth, mipLevels, arrayLevels, samples, tiling, usageFlags, sharingMode, imageLayout,flags);
	}

	TextureView* GraphicsDevice::CreateTextureView(const Texture* pTexture, const unsigned int mipIndex, const unsigned int arrayIndex, const VkImageViewType viewType, const VkFormat format, const VkComponentMapping mapping, const VkImageAspectFlags aspectMask)
	{
		return new TextureView(this,pTexture,mipIndex,arrayIndex,viewType,format,mapping,aspectMask);
	}
	Fence* GraphicsDevice::CreateFence(bool bSignalled)
	{
		return new Fence(this,bSignalled);
	}
	Semaphore* GraphicsDevice::CreateSemaphore(bool bSignalled)
	{
		return new Semaphore(this,bSignalled);
	}
	CommandPool* GraphicsDevice::CreateCommandPool(const VkQueueFlags flags)
	{
		return new CommandPool(this,flags);
	}
	CommandList* GraphicsDevice::AllocateCommandList(const CommandPool* pPool)
	{
		return new CommandList(this,pPool);
	}
	Shader* GraphicsDevice::CreateShader(const std::string& entryPoint, const VkShaderStageFlags stage, const unsigned char* pBytes, const unsigned int byteCount)
	{
		return new Shader(this,entryPoint,stage,pBytes,byteCount);
	}
	GraphicsBuffer* GraphicsDevice::CreateBuffer(GraphicsMemory* pMemory, const VkBufferUsageFlags usages, const VkSharingMode sharingMode, const unsigned long long bufferSize)
	{
		return new GraphicsBuffer(this,pMemory,usages,sharingMode,bufferSize);
	}
	GraphicsPipeline* GraphicsDevice::CreateGraphicsPipeline(
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
		const void* pNext)
	{
		return new GraphicsPipeline(
			this,
			vertexInputState,
			inputAssemblyState,
			pTesellationState,
			pViewportState,
			rasterizerState,
			multisampleState,
			depthStencilState,
			blendingState,
			pDynamicState,
			ppShaders, shaderCount,
			ppDescriptorLayouts, descriptorLayoutCount,
			pRenderPass,
			pBasePipeline, basePipelineIndex,
			pNext);
	}
	ComputePipeline* GraphicsDevice::CreateComputePipeline(
		const VkPipelineCreateFlags flags,
		const Shader* pShader,
		const DescriptorSetLayout** ppSetLayouts, const unsigned int setLayoutCount,
		const Pipeline* pBasePipeline, const unsigned int basePipelineIndex)
	{
		return new ComputePipeline(this,flags,pShader,ppSetLayouts,setLayoutCount,pBasePipeline,basePipelineIndex);
	}
	Sampler* GraphicsDevice::CreateSampler(
		const VkFilter magFilter, const VkFilter minFilter,
		const VkSamplerMipmapMode mipmapMode, const VkSamplerAddressMode u, const VkSamplerAddressMode v, const VkSamplerAddressMode w,
		const float mipLodBias,
		const VkBool32 bAnisotropyEnabled, const float maxAnisotropy,
		const VkBool32 bCompareEnabled, const VkCompareOp compareOp,
		const float minLod, const float maxLod,
		const VkBorderColor borderColor,
		const VkBool32 unnormalizedCoordinates)
	{
		return new Sampler(this,magFilter,minFilter,mipmapMode,u,v,w,mipLodBias,bAnisotropyEnabled,maxAnisotropy,bCompareEnabled,compareOp,minLod,maxLod,borderColor,unnormalizedCoordinates);
	}
	DescriptorPool* GraphicsDevice::CreateDescriptorPool(
		const VkDescriptorPoolCreateFlags flags,
		const unsigned int maxSets,
		const VkDescriptorType* pDescriptorTypes, const unsigned int* pDescriptorCounts,
		const unsigned int descriptorTypeCount)
	{
		return new DescriptorPool(this,flags,maxSets,pDescriptorTypes,pDescriptorCounts,descriptorTypeCount);
	}
	DescriptorSetLayout* GraphicsDevice::CreateDescriptorSetLayout(
		const unsigned int* pBindings,
		const VkDescriptorType* pDescriptorTypes,
		const unsigned int* pDescriptorCounts,
		const VkShaderStageFlags* pStageFlags,
		const unsigned char bindingCount)
	{
		return new DescriptorSetLayout(this,pBindings,pDescriptorTypes,pDescriptorCounts,pStageFlags,bindingCount);
	}
	DescriptorSet* GraphicsDevice::AllocateDescriptorSet(const DescriptorPool* pPool, const DescriptorSetLayout* pLayout)
	{
		return new DescriptorSet(this,pPool,pLayout);
	}
	void GraphicsDevice::UpdateHostBuffer(GraphicsBuffer* pBuffer, const unsigned char* pData, const unsigned long long dataSize, const unsigned long long dstBufferOffset)
	{
		void* pTargetHostData = nullptr;
		DEV_ASSERT(vkMapMemory(mLogicalDevice, pBuffer->GetMemory()->GetMemory(), pBuffer->GetMemoryAlignedOffset() + dstBufferOffset, dataSize, VkMemoryMapFlags(), &pTargetHostData) == VK_SUCCESS, "GraphicsDevice", "Failed to map host buffer");
		memcpy(pTargetHostData, pData, dataSize);
		vkUnmapMemory(mLogicalDevice, pBuffer->GetMemory()->GetMemory());
	}
	void GraphicsDevice::UpdateDescriptorSetTextureSampler(
		const DescriptorSet* pSet,
		const unsigned int dstBinding,
		const unsigned int dstArrayElement,
		const VkDescriptorType type,
		const TextureView* pTextureView,
		const Sampler* pSampler,
		const VkImageLayout textureLayout)
	{
		VkDescriptorImageInfo imageInfo = {};
		imageInfo.imageLayout = textureLayout;
		imageInfo.imageView = pTextureView != nullptr ? pTextureView->GetView() : VK_NULL_HANDLE;
		imageInfo.sampler = pSampler != nullptr ? pSampler->GetSampler() : VK_NULL_HANDLE;

		VkWriteDescriptorSet writeInfo = {};
		writeInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeInfo.dstSet = pSet->GetSet();
		writeInfo.dstBinding = dstBinding;
		writeInfo.dstArrayElement = dstArrayElement;
		writeInfo.descriptorCount = 1;
		writeInfo.descriptorType = type;
		writeInfo.pImageInfo = &imageInfo;
		writeInfo.pNext = nullptr;

		vkUpdateDescriptorSets(mLogicalDevice, 1, &writeInfo, 0, nullptr);
	}
	void GraphicsDevice::UpdateDescriptorSetBuffer(const DescriptorSet* pSet, const unsigned int dstBinding, const unsigned int dstArrayElement, const VkDescriptorType type, const GraphicsBuffer* pBuffer, const unsigned long long offset, const unsigned long long range)
	{
		VkDescriptorBufferInfo bufferInfo = {};
		bufferInfo.buffer = pBuffer->GetBuffer();
		bufferInfo.offset = offset;
		bufferInfo.range = range;

		VkWriteDescriptorSet writeInfo = {};
		writeInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeInfo.dstSet = pSet->GetSet();
		writeInfo.dstBinding = dstBinding;
		writeInfo.dstArrayElement = dstArrayElement;
		writeInfo.descriptorCount = 1;
		writeInfo.descriptorType = type;
		writeInfo.pBufferInfo = &bufferInfo;
		writeInfo.pNext = nullptr;

		vkUpdateDescriptorSets(mLogicalDevice, 1, &writeInfo, 0, nullptr);
	}
	void GraphicsDevice::SubmitCommandLists(const GraphicsQueue* pQueue, const CommandList** ppCmdLists, const unsigned int cmdListCount, const VkPipelineStageFlags* pWaitDstPipelineStageFlags, const Fence* pWaitFence, const Semaphore** ppWaitSemaphores, const unsigned int waitSemaphoreCount, const Semaphore** ppSignalSemaphores, const unsigned int signalSemaphoreCount)
	{
		//Collect command buffers
		VkCommandBuffer cmdBuffers[32];
		for (unsigned int i = 0; i < cmdListCount; i++)
		{
			const CommandList* pCmdList = ppCmdLists[i];
			cmdBuffers[i] = pCmdList->GetCmdBuffer();
		}

		//Collect wait semaphores
		VkSemaphore waitSemaphores[32];
		for (unsigned int i = 0; i < waitSemaphoreCount; i++)
		{
			const Semaphore* pSemaphore = ppWaitSemaphores[i];
			waitSemaphores[i] = pSemaphore->GetSemaphore();

		}

		//Collect signal semaphores
		VkSemaphore signalSemaphores[32];
		for (unsigned int i = 0; i < signalSemaphoreCount; i++)
		{
			const Semaphore* pSemaphore = ppSignalSemaphores[i];
			signalSemaphores[i] = pSemaphore->GetSemaphore();
		}

		//Create submit info
		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		submitInfo.waitSemaphoreCount = waitSemaphoreCount;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.signalSemaphoreCount = signalSemaphoreCount;
		submitInfo.pSignalSemaphores = signalSemaphores;

		submitInfo.commandBufferCount = cmdListCount;
		submitInfo.pCommandBuffers = cmdBuffers;

		submitInfo.pWaitDstStageMask = pWaitDstPipelineStageFlags;

		submitInfo.pNext = nullptr;

		DEV_ASSERT(vkQueueSubmit(pQueue->GetQueue(), 1, &submitInfo, pWaitFence != nullptr ? pWaitFence->GetFence() : nullptr) == VK_SUCCESS,"GraphicsDevice","Failed to submit to queue");
	}

	GraphicsDevice::GraphicsDevice(const std::vector<std::string>& extensions, const std::vector<std::string>& layers, const std::vector<QueueFamilyRequestInfo>& queueFamilyRequests, const VkPhysicalDeviceFeatures* pFeatures, const void* pDeviceNext, GraphicsAdapter* pAdapter) : mPhysicalDevice(pAdapter->GetPhysicalDevice()), mLogicalDevice(VK_NULL_HANDLE),mOwnerAdapter(pAdapter)
	{
		//Pre validations
		DEV_ASSERT(pAdapter != nullptr,"GraphicsDevice","Invalid graphics adapter!");
		DEV_ASSERT(queueFamilyRequests.size() > 0, "GraphicsDevice", "Cannot have 0 queue family requests!");

		//Get requested extensions
		std::vector<std::string> requestedExtensions;
		for (const std::string& extension : extensions)
			requestedExtensions.push_back(extension);

		//Get supported extension count
		unsigned int supportedExtensionCount = 0;
		DEV_ASSERT(vkEnumerateDeviceExtensionProperties(mPhysicalDevice, nullptr, &supportedExtensionCount, nullptr) == VK_SUCCESS, "GraphicsDevice", "Failed to get the supported extension count");
		DEV_ASSERT(supportedExtensionCount > 0, "GraphicsDevice", "No extensions found");

		std::vector<VkExtensionProperties> supportedExtensions(supportedExtensionCount);
		DEV_ASSERT(vkEnumerateDeviceExtensionProperties(mPhysicalDevice, nullptr, &supportedExtensionCount, supportedExtensions.data()) == VK_SUCCESS, "GraphicsDevice", "Failed to get the supported extensions");

		std::vector<const char*> validatedExtensions;
		for (const std::string& requestedExtension : requestedExtensions)
		{
			for (const VkExtensionProperties& extension : supportedExtensions)
			{
				if (strcmp(requestedExtension.c_str(), extension.extensionName) == 0)
				{
					validatedExtensions.push_back(requestedExtension.c_str());
					break;
				}
			}
		}
		DEV_ASSERT(requestedExtensions.size() == validatedExtensions.size(), "GraphicsDevice", "Some of the requested extensions are not supported");

		//Get requested layers
		std::vector<std::string> requestedLayers;
		for (const std::string& layer : layers)
			requestedLayers.push_back(layer);

		//Get supported layer count
		unsigned int supportedLayerCount = 0;
		DEV_ASSERT(vkEnumerateDeviceLayerProperties(mPhysicalDevice, &supportedLayerCount, nullptr) == VK_SUCCESS, "GraphicsDevice", "Failed to get the supported layer count");
		DEV_ASSERT(supportedLayerCount > 0, "GraphicsDevice", "No layers found");

		std::vector<VkLayerProperties> supportedLayers(supportedLayerCount);
		DEV_ASSERT(vkEnumerateDeviceLayerProperties(mPhysicalDevice, &supportedLayerCount, supportedLayers.data()) == VK_SUCCESS, "GraphicsDevice", "Failed to get the supported layers");

		std::vector<const char*> validatedLayers;
		for (const std::string& requestedLayer : requestedLayers)
		{
			for (const VkLayerProperties& layer : supportedLayers)
			{
				if (strcmp(requestedLayer.c_str(), layer.layerName) == 0)
				{
					validatedLayers.push_back(requestedLayer.c_str());
					break;
				}
			}
		}
		DEV_ASSERT(validatedLayers.size() == requestedLayers.size(), "GraphicsDevice", "Some of the requested layers are not supported");

		//Get queue family count
		unsigned int queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(mPhysicalDevice, &queueFamilyCount, nullptr);
		DEV_ASSERT(queueFamilyCount > 0, "GraphicsDevice", "No queue families found!?");
		DEV_ASSERT(queueFamilyRequests.size() <= queueFamilyCount, "GraphicsDevice", "Some of the requested queue families are not supported");

		//Get queue family properties
		std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(mPhysicalDevice, &queueFamilyCount, queueFamilyProperties.data());

		//Get queue families
		for (const QueueFamilyRequestInfo& queueFamilyRequestInfo : queueFamilyRequests)
		{
			unsigned int familyIndex = 0;
			for (const VkQueueFamilyProperties& queueFamilyProperties : queueFamilyProperties)
			{
				if (queueFamilyProperties.queueFlags & queueFamilyRequestInfo.Flags)
				{
					mQueueFamilies.push_back(DeviceQueueFamily(queueFamilyRequestInfo.Flags, familyIndex,queueFamilyRequestInfo.Count));
					break;
				}
				familyIndex++;
			}
		}
		DEV_ASSERT(mQueueFamilies.size() >= queueFamilyRequests.size(), "GraphicsDevice", "Some of the requested queue families are not supported!");

		//Get queue create informations
		constexpr float queuePriorities[] = { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f };
		std::vector<VkDeviceQueueCreateInfo> queueCreateInformations;
		for (const DeviceQueueFamily& queueFamily : mQueueFamilies)
		{
			VkDeviceQueueCreateInfo queueInfo = {};
			queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueInfo.queueFamilyIndex = queueFamily.GetFamilyIndex();
			queueInfo.queueCount = queueFamily.GetRequestedCount();
			queueInfo.pQueuePriorities = queuePriorities;
			queueInfo.pNext = nullptr;

			queueCreateInformations.push_back(queueInfo);
		}

		//Create logical device
		VkDeviceCreateInfo deviceInfo = {};
		deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceInfo.queueCreateInfoCount = queueCreateInformations.size();
		deviceInfo.pQueueCreateInfos = queueCreateInformations.data();
		deviceInfo.enabledExtensionCount = validatedExtensions.size();
		deviceInfo.ppEnabledExtensionNames = validatedExtensions.data();
		deviceInfo.enabledLayerCount = validatedLayers.size();
		deviceInfo.ppEnabledLayerNames = validatedLayers.data();
		deviceInfo.pEnabledFeatures = pFeatures;
		deviceInfo.pNext = pDeviceNext;

		DEV_ASSERT(vkCreateDevice(mPhysicalDevice, &deviceInfo, nullptr, &mLogicalDevice) == VK_SUCCESS, "GraphicsDevice", "Failed to create logical device");

		//Get queues
		for (DeviceQueueFamily& family : mQueueFamilies)
		{
			for (unsigned int i = 0; i < family.GetRequestedCount(); i++)
			{
				VkQueue queue = VK_NULL_HANDLE;
				vkGetDeviceQueue(mLogicalDevice, family.GetFamilyIndex(), i, &queue);
				family.ReturnQueue(queue);
			}
		}
	}
	void GraphicsDevice::SignalQueueReturned(const VkQueue queue, const unsigned int familyIndex)
	{
		for (DeviceQueueFamily& family : mQueueFamilies)
		{
			if (family.GetFamilyIndex() == familyIndex)
			{
				family.ReturnQueue(queue);
				return;
			}
		}
	}
	GraphicsDevice::~GraphicsDevice()
	{
		//Destroy queues
		vkDeviceWaitIdle(mLogicalDevice); // waits for all the queues
		vkDestroyDevice(mLogicalDevice, nullptr);

		//Inform adapter
	}
}
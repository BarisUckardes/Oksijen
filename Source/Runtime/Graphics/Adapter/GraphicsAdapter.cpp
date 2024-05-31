#include "GraphicsAdapter.h"
#include <Runtime/Graphics/Surface/Surface.h>
#include <Runtime/Graphics/Queue/GraphicsQueue.h>

namespace Oksijen
{
	GraphicsDevice* GraphicsAdapter::CreateDevice(const std::vector<std::string>& extensions, const std::vector<std::string>& layers, const std::vector<QueueFamilyRequestInfo>& queueFamilyRequests, const VkPhysicalDeviceFeatures* pFeatures, const void* pDeviceNext)
	{
		GraphicsDevice* pDevice = new GraphicsDevice(extensions, layers, queueFamilyRequests, pFeatures, pDeviceNext, this);
		return pDevice;
	}
	bool GraphicsAdapter::IsQueueSupportsSurface(const GraphicsQueue* pQueue, const Surface* pSurface)
	{
		DEV_ASSERT(pQueue != nullptr, "GraphicsAdapter", "Invalid queue");
		DEV_ASSERT(pSurface != nullptr, "GraphicsAdapter", "Invalid surface");

		VkBool32 bSupported = false;
		DEV_ASSERT(vkGetPhysicalDeviceSurfaceSupportKHR(mPhysicalDevice, pQueue->GetFamilyIndex(), pSurface->GetSurface(), &bSupported) == VK_SUCCESS, "GraphicsAdapter", "Failed to query for surface support");
		return bSupported;
	}
	VkSurfaceCapabilitiesKHR GraphicsAdapter::GetSurfaceCapabilities(const Surface* pSurface)
	{
		DEV_ASSERT(pSurface != nullptr, "GraphicsAdapter", "Invalid surface");

		VkSurfaceCapabilitiesKHR capabilities = {};
		DEV_ASSERT(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(mPhysicalDevice, pSurface->GetSurface(), &capabilities) == VK_SUCCESS, "GraphicsAdapter", "Failed to query for surface capabilities");
		return capabilities;
	}
	std::vector<VkSurfaceFormatKHR> GraphicsAdapter::GetSurfaceSupportedFormats(const Surface* pSurface)
	{
		DEV_ASSERT(pSurface != nullptr, "GraphicsAdapter", "Invalid surface");
		unsigned int supportedFormatCount = 0;
		DEV_ASSERT(vkGetPhysicalDeviceSurfaceFormatsKHR(mPhysicalDevice, pSurface->GetSurface(), &supportedFormatCount, nullptr) == VK_SUCCESS, "GraphicsAdapter", "Failed to get the supported surface format count");
		DEV_ASSERT(supportedFormatCount > 0, "GraphicsAdapter", "No surface formats found");

		std::vector<VkSurfaceFormatKHR> supportedFormats(supportedFormatCount);
		DEV_ASSERT(vkGetPhysicalDeviceSurfaceFormatsKHR(mPhysicalDevice, pSurface->GetSurface(), &supportedFormatCount, supportedFormats.data()) == VK_SUCCESS, "GraphicsAdapter", "Failed to get the supported surface formats");

		return supportedFormats;
	}
	std::vector<VkPresentModeKHR> GraphicsAdapter::GetSurfaceSupportedPresentModes(const Surface* pSurface)
	{
		DEV_ASSERT(pSurface != nullptr, "GraphicsAdapter", "Invalid surface");

		unsigned int supportedPresentModeCount = 0;
		DEV_ASSERT(vkGetPhysicalDeviceSurfacePresentModesKHR(mPhysicalDevice, pSurface->GetSurface(), &supportedPresentModeCount, nullptr) == VK_SUCCESS, "GraphicsAdapter", "Failed to get the supported present mode count");
		DEV_ASSERT(supportedPresentModeCount > 0, "GraphicsAdapter", "No surface present modes found");

		std::vector<VkPresentModeKHR> supportedPresentModes(supportedPresentModeCount);
		DEV_ASSERT(vkGetPhysicalDeviceSurfacePresentModesKHR(mPhysicalDevice, pSurface->GetSurface(), &supportedPresentModeCount, supportedPresentModes.data()) == VK_SUCCESS, "GraphicsAdapter", "Failed to get the supported present modes");

		return supportedPresentModes;
	}
}
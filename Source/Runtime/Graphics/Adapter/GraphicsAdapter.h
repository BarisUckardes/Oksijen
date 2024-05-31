#pragma once
#include <Runtime/Graphics/Device/GraphicsDevice.h>
#include <Runtime/Core/Core.h>
#include <vulkan/vulkan.h>
#include <string>
#include <vector>

namespace Oksijen
{
	class Surface;
	class GraphicsQueue;
	class RUNTIME_API GraphicsAdapter final
	{
	public:
		GraphicsAdapter(const VkPhysicalDevice physicalDevice) : mPhysicalDevice(physicalDevice)
		{

		}
		~GraphicsAdapter()
		{

		}

		FORCEINLINE VkPhysicalDevice GetPhysicalDevice() const noexcept { return mPhysicalDevice; }

		GraphicsDevice* CreateDevice(const std::vector<std::string>& extensions, const std::vector<std::string>& layers, const std::vector<QueueFamilyRequestInfo>& queueFamilyRequests, const VkPhysicalDeviceFeatures* pFeatures, const void* pDeviceNext);


		bool IsQueueSupportsSurface(const GraphicsQueue* pQueue, const Surface* pSurface);
		VkSurfaceCapabilitiesKHR GetSurfaceCapabilities(const Surface* pSurface);
		std::vector<VkSurfaceFormatKHR> GetSurfaceSupportedFormats(const Surface* pSurface);
		std::vector<VkPresentModeKHR> GetSurfaceSupportedPresentModes(const Surface* pSurface);
	private:
		const VkPhysicalDevice mPhysicalDevice;
	};
}
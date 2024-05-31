#pragma once
#include <Runtime/Graphics/Adapter/GraphicsAdapter.h>
#include <Runtime/Graphics/Surface/Surface.h>
#include <vulkan/vulkan.h>
#include <string>
#include <vector>

namespace Oksijen
{
	class RUNTIME_API GraphicsInstance final
	{
	public:
		GraphicsInstance(const std::string& appName, const std::string& engineName, const unsigned int appVersion, const unsigned int apiVersion, const std::vector<std::string>& extensions, const std::vector<std::string>& layers);
		~GraphicsInstance();

		FORCEINLINE VkInstance GetInstance() const noexcept { return mInstance; }
		FORCEINLINE GraphicsAdapter* GetDefaultAdapter() const noexcept { return mAdapters[0]; }
		FORCEINLINE const std::vector<GraphicsAdapter*> GetAdapters() const noexcept { return mAdapters; }

		Surface* CreateSurface(const PlatformWindow* pWindow);
	private:
		VkInstance mInstance;
		std::vector<GraphicsAdapter*> mAdapters;
#ifdef OKSIJEN_DEBUG
		VkDebugUtilsMessengerEXT mDebugMessenger;
#endif
	};
}
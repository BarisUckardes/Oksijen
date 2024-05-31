#pragma once
#include <Runtime/Platform/PlatformWindow.h>
#include <Runtime/Core/Core.h>
#include <vulkan/vulkan.h>

namespace Oksijen
{
	class GraphicsDevice;
	class GraphicsInstance;
	class RUNTIME_API Surface final
	{
	public:
		Surface(const PlatformWindow* pWindow,const GraphicsInstance* pInstance);
		~Surface();

		FORCEINLINE VkSurfaceKHR GetSurface() const noexcept { return mSurface; }
	private:
		const VkInstance mInstance;
		VkSurfaceKHR mSurface;
	};
}
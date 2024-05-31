#include "Surface.h"
#include <Runtime/Graphics/Device/GraphicsDevice.h>
#include <Runtime/Graphics/Instance/GraphicsInstance.h>

#ifdef OKSIJEN_PLATFORM_WINDOWS
#include <Windows.h>
#include <vulkan/vulkan_win32.h>
#include <Runtime/Windows/Win32Window.h>
#endif

namespace Oksijen
{
	Surface::Surface(const PlatformWindow* pWindow, const GraphicsInstance* pInstance) : mInstance(pInstance->GetInstance())
	{
#ifdef OKSIJEN_PLATFORM_WINDOWS
		const Win32Window* pWin32Window = (const Win32Window*)pWindow;

		VkWin32SurfaceCreateInfoKHR surfaceInfo = {};
		surfaceInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		surfaceInfo.hinstance = GetModuleHandle(NULL);
		surfaceInfo.hwnd = pWin32Window->GetWin32Handle();
		surfaceInfo.flags = VkWin32SurfaceCreateFlagsKHR();
		surfaceInfo.pNext = nullptr;

		DEV_ASSERT(vkCreateWin32SurfaceKHR(mInstance,&surfaceInfo,nullptr,&mSurface) == VK_SUCCESS,"Surface","Failed to create win32 surface");
#endif
	}
	Surface::~Surface()
	{
		vkDestroySurfaceKHR(mInstance, mSurface, nullptr);
	}
}
#include <stdio.h>
#include <Runtime/Platform/PlatformWindow.h>
#include <Runtime/Graphics/Instance/GraphicsInstance.h>
#include <Runtime/Graphics/Device/GraphicsDevice.h>

#ifdef OKSIJEN_PLATFORM_WINDOWS
#include <Windows.h>
#include <vulkan/vulkan_win32.h>
#endif
#include <Runtime/Platform/PlatformMonitor.h>

namespace Oksijen
{
	void Run()
	{
		//Get monitor
		PlatformMonitor* pMonitor = PlatformMonitor::GetMonitors()[1];

		//Create window
		WindowDesc windowDesc = {};
		windowDesc.X = 0;
		windowDesc.Y = 0;
		windowDesc.Width = 512;
		windowDesc.Height = 512;
		windowDesc.Title = "Oksijen_RedBlueAlternating";

		PlatformWindow* pWindow = PlatformWindow::Create(windowDesc);
		pWindow->Show();

		//Set target monitor dettails
		pWindow->SetMode(WindowMode::Borderless);
		pWindow->SetPosition(pMonitor->GetPosX(), pMonitor->GetPosY());
		pWindow->SetSize(pMonitor->GetWidth(), pMonitor->GetHeight());

		//Create instance
		std::vector<std::string> instanceExtensions;
		std::vector<std::string> instanceLayers;

		instanceExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
		instanceExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

#ifdef OKSIJEN_PLATFORM_WINDOWS
		instanceExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#endif

		GraphicsInstance* pInstance = new GraphicsInstance("RedBlueAlternating", "Runtime", VK_MAKE_VERSION(1, 0, 0), VK_API_VERSION_1_3, instanceExtensions, instanceLayers);

		//Get adapter
		GraphicsAdapter* pDefaultAdapter = pInstance->GetDefaultAdapter();

		//Create device
		std::vector<std::string> deviceExtensions;
		deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

		std::vector<std::string> deviceLayers;
		std::vector<QueueFamilyRequestInfo> deviceQueueFamilyRequests;
		deviceQueueFamilyRequests.push_back({ VK_QUEUE_GRAPHICS_BIT,1 });

		//Create device
		VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamicRenderingFeatures = {};
		dynamicRenderingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR;
		dynamicRenderingFeatures.dynamicRendering = VK_TRUE;
		dynamicRenderingFeatures.pNext = nullptr;
		GraphicsDevice* pDevice = pDefaultAdapter->CreateDevice(deviceExtensions, deviceLayers, deviceQueueFamilyRequests, nullptr, &dynamicRenderingFeatures);

		//Get default graphics queue
		GraphicsQueue* pDefaultGraphicsQueue = pDevice->RentQueue(VK_QUEUE_GRAPHICS_BIT);

		//Create surface
		Surface* pSurface = pInstance->CreateSurface(pWindow);

		//Test surface
		constexpr unsigned int requestedSwapchainBufferCount = 2;
		constexpr VkFormat requestedSwapchainFormat = VK_FORMAT_R8G8B8A8_UNORM;
		constexpr VkImageUsageFlags requestedSwapchainImageUsageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		constexpr VkColorSpaceKHR requestedSwapchainFormatColorspace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
		constexpr VkPresentModeKHR requestedSwapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;
		const unsigned int requestedSwapchainWidth = pWindow->GetWidth();
		const unsigned int requestedSwapchainHeight = pWindow->GetHeight();
		{
			DEV_ASSERT(pDefaultAdapter->IsQueueSupportsSurface(pDefaultGraphicsQueue, pSurface), "Main", "Queue does not supported this surface");


			const VkSurfaceCapabilitiesKHR capabilities = pDefaultAdapter->GetSurfaceCapabilities(pSurface);
			DEV_ASSERT(capabilities.minImageCount <= requestedSwapchainBufferCount && capabilities.maxImageCount >= requestedSwapchainBufferCount, "Main", "Invalid buffer count requested");
			DEV_ASSERT(capabilities.supportedUsageFlags & requestedSwapchainImageUsageFlags, "Main", "Invalid image usage flags requested");
			DEV_ASSERT(capabilities.minImageExtent.width <= requestedSwapchainWidth && capabilities.maxImageExtent.width >= requestedSwapchainWidth, "Main", "Invalid swapchain width requested");
			DEV_ASSERT(capabilities.minImageExtent.height <= requestedSwapchainHeight && capabilities.maxImageExtent.height >= requestedSwapchainHeight, "Main", "Invalid swapchain height requested");
			const std::vector<VkSurfaceFormatKHR> supportedFormats = pDefaultAdapter->GetSurfaceSupportedFormats(pSurface);
			{
				bool bSupported = false;
				for (const VkSurfaceFormatKHR& format : supportedFormats)
				{
					if (format.format == requestedSwapchainFormat && format.colorSpace == requestedSwapchainFormatColorspace)
					{
						bSupported = true;
						break;
					}
				}
				DEV_ASSERT(bSupported, "Main", "Invalid swapchain format or colorspace requested");
			}
			
			const std::vector<VkPresentModeKHR> supportedPresentModes = pDefaultAdapter->GetSurfaceSupportedPresentModes(pSurface);
			{
				bool bSupported = false;
				for (const VkPresentModeKHR presentMode : supportedPresentModes)
				{
					if (presentMode == requestedSwapchainPresentMode)
					{
						bSupported = true;
						break;
					}
				}
				DEV_ASSERT(bSupported, "Main", "Invalid swapchain present mode requested!");
			}

		}

		//Create swapchain
		Swapchain* pSwapchain = pDevice->CreateSwapchain(
			pSurface,
			pDefaultGraphicsQueue,
			requestedSwapchainBufferCount,
			requestedSwapchainWidth, requestedSwapchainHeight,
			1,
			requestedSwapchainPresentMode,
			requestedSwapchainFormat, requestedSwapchainFormatColorspace,
			requestedSwapchainImageUsageFlags);

		//Create swapchain texture views
		TextureView* ppSwapchainTextureViews[requestedSwapchainBufferCount];
		for (unsigned int i = 0; i < requestedSwapchainBufferCount; i++)
		{
			constexpr VkComponentMapping mapping =
			{
				VK_COMPONENT_SWIZZLE_IDENTITY,
				VK_COMPONENT_SWIZZLE_IDENTITY,
				VK_COMPONENT_SWIZZLE_IDENTITY,
				VK_COMPONENT_SWIZZLE_IDENTITY 
			};
			ppSwapchainTextureViews[i] = pDevice->CreateTextureView(pSwapchain->GetTexture(i), 0, 0, VK_IMAGE_VIEW_TYPE_2D, requestedSwapchainFormat, mapping, VK_IMAGE_ASPECT_COLOR_BIT);
		}

		//Create fences
		Fence* pPresentImageAcquireFence = pDevice->CreateFence(false);
		Fence* pCmdFence = pDevice->CreateFence(false);

		//Create command pool
		CommandPool* pCmdPool = pDevice->CreateCommandPool(VK_QUEUE_GRAPHICS_BIT);

		//Create command list
		CommandList* pCmdList = pDevice->AllocateCommandList(pCmdPool);

		//Set pipeline barriers for the images
		pCmdList->Begin();
		for (unsigned char i = 0; i < requestedSwapchainBufferCount; i++)
		{
			pCmdList->SetPipelineTextureBarrier(
				pSwapchain->GetTexture(i),
				VK_IMAGE_LAYOUT_UNDEFINED,VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
				VK_QUEUE_GRAPHICS_BIT,VK_QUEUE_GRAPHICS_BIT,
				VK_IMAGE_ASPECT_COLOR_BIT,0,0,
				VK_ACCESS_NONE,VK_ACCESS_COLOR_ATTACHMENT_READ_BIT,
				VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				VkDependencyFlags());
		}
		pCmdList->End();
		pDevice->SubmitCommandLists(pDefaultGraphicsQueue, (const CommandList**) & pCmdList, 1, nullptr, pCmdFence, nullptr, 0, nullptr, 0);
		pCmdFence->Wait();
		pCmdFence->Reset();

		bool bRed = true;
		while (pWindow->IsActive())
		{
			// poll window events
			pWindow->PollEvents();
			if (!pWindow->IsActive())
			{
				break;
			}

			//Acquire image index
			const unsigned int swapchainImageIndex = pSwapchain->AcquireImageIndex(pPresentImageAcquireFence, nullptr);
			pPresentImageAcquireFence->Wait();
			pPresentImageAcquireFence->Reset();

			//Begin cmd
			pCmdList->Begin();

			//Pre barrier
			pCmdList->SetPipelineTextureBarrier(
				pSwapchain->GetTexture(swapchainImageIndex),
				VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				VK_QUEUE_GRAPHICS_BIT, VK_QUEUE_GRAPHICS_BIT,
				VK_IMAGE_ASPECT_COLOR_BIT, 0, 0,
				VK_ACCESS_COLOR_ATTACHMENT_READ_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				VkDependencyFlags());


			//Set dynamic rendering color attachment
			VkClearValue colorAttachmentClearValue = {};
			if (bRed)
			{
				colorAttachmentClearValue.color.float32[0] = 1.0f;
				colorAttachmentClearValue.color.float32[1] = 0.0f;
				colorAttachmentClearValue.color.float32[2] = 0.0f;
				colorAttachmentClearValue.color.float32[3] = 1.0f;
			}
			else
			{
				colorAttachmentClearValue.color.float32[0] = 0.0f;
				colorAttachmentClearValue.color.float32[1] = 0.0f;
				colorAttachmentClearValue.color.float32[2] = 1.0f;
				colorAttachmentClearValue.color.float32[3] = 1.0f;
			}

			pCmdList->AddDynamicRenderingColorAttachment(
				ppSwapchainTextureViews[swapchainImageIndex],
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				VkResolveModeFlags(),
				nullptr,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_ATTACHMENT_LOAD_OP_CLEAR,
				VK_ATTACHMENT_STORE_OP_STORE,
				colorAttachmentClearValue);

			pCmdList->BeginDynamicRendering(0,1,0, 0, pWindow->GetWidth(), pWindow->GetHeight());

			//End dynamic rendering
			pCmdList->EndDynamicRendering();

			//Post barrier for presenting
			pCmdList->SetPipelineTextureBarrier(
				pSwapchain->GetTexture(swapchainImageIndex),
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
				VK_QUEUE_GRAPHICS_BIT, VK_QUEUE_GRAPHICS_BIT,
				VK_IMAGE_ASPECT_COLOR_BIT, 0, 0,
				VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_COLOR_ATTACHMENT_READ_BIT,
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				VkDependencyFlags());

			//End cmd
			pCmdList->End();

			//Submit cmd list
			pDevice->SubmitCommandLists(pDefaultGraphicsQueue, (const CommandList**)&pCmdList, 1, nullptr, pCmdFence, nullptr, 0, nullptr, 0);
			pCmdFence->Wait();
			pCmdFence->Reset();

			//Present
			pSwapchain->Present(swapchainImageIndex);

			bRed = !bRed;
		}
	}
}
int main(const unsigned int argumentCount, const char** ppArguments)
{
	Oksijen::Run();
	return 0;
}
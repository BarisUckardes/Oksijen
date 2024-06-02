#include <stdio.h>
#include <Runtime/Platform/PlatformWindow.h>
#include <Runtime/Graphics/Instance/GraphicsInstance.h>
#include <Runtime/Graphics/Device/GraphicsDevice.h>

#ifdef OKSIJEN_PLATFORM_WINDOWS
#include <Windows.h>
#include <vulkan/vulkan_win32.h>
#endif
#include <Runtime/Platform/PlatformMonitor.h>
#include <Runtime/Graphics/Shader/ShaderCompiler.h>
#include <Runtime/Graphics/Texture/TextureLoader.h>

namespace Oksijen
{
	static const char computeShaderSource[] =
		R"STR(
			#version 430 core
			layout(local_size_x = 10,local_size_y = 10,local_size_z =1) in;
			layout(rgba32f,binding = 0) uniform image2D imgOutput;
			void main()
			{
				imageStore(imgOutput,ivec2(gl_GlobalInvocationID.xy),vec4(0,1.0f,0,1.0f));
			}	  
)STR";
	

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
		windowDesc.Title = "06_ComputeTexture";

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
		deviceExtensions.push_back(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME);

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
		GraphicsQueue* pDefaultQueue = pDevice->RentQueue(VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT);

		//Create command pool
		CommandPool* pCmdPool = pDevice->CreateCommandPool(VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT);

		//Create command list
		CommandList* pCmdList = pDevice->AllocateCommandList(pCmdPool);

		//Create general use-case fence
		Fence* pCmdFence = pDevice->CreateFence(false);

		//Create surface
		Surface* pSurface = pInstance->CreateSurface(pWindow);

		//Test surface
		const unsigned int requestedSwapchainBufferCount = 2;
		const VkFormat requestedSwapchainFormat = VK_FORMAT_R8G8B8A8_UNORM;
		const VkImageUsageFlags requestedSwapchainImageUsageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
		const VkColorSpaceKHR requestedSwapchainFormatColorspace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
		const VkPresentModeKHR requestedSwapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;
		const unsigned int requestedSwapchainWidth = pWindow->GetWidth();
		const unsigned int requestedSwapchainHeight = pWindow->GetHeight();
		{
			DEV_ASSERT(pDefaultAdapter->IsQueueSupportsSurface(pDefaultQueue, pSurface), "Main", "Queue does not supported this surface");


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
			pDefaultQueue,
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

		//Create present image fence
		Fence* pPresentImageAcquireFence = pDevice->CreateFence(false);


		//Compile shaders
		unsigned char* pComputeShaderSPIRVBytes = nullptr;
		unsigned int computeShaderSPIRVByteCount = 0;
		std::string computeShaderCompilationErrors;
		DEV_ASSERT(ShaderCompiler::TextToSPIRV(computeShaderSource, "main", shaderc_compute_shader, shaderc_source_language_glsl, &pComputeShaderSPIRVBytes, computeShaderSPIRVByteCount, computeShaderCompilationErrors), "Main", "Failed to compile compute shader! with logs: %s", computeShaderCompilationErrors.c_str());

		//Create shaders
		Shader* pComputeShader = pDevice->CreateShader("main", VK_SHADER_STAGE_COMPUTE_BIT, pComputeShaderSPIRVBytes, computeShaderSPIRVByteCount);

		//Create descriptor pool
		constexpr VkDescriptorType poolDescriptorTypes[] = { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE };
		constexpr unsigned int poolDescriptorTypeCounts[] = { 3 };
		DescriptorPool* pDescriptorPool = pDevice->CreateDescriptorPool(
			VkDescriptorPoolCreateFlags(),
			3,
			poolDescriptorTypes, poolDescriptorTypeCounts,
			1
		);

		//Create descriptor set layout
		constexpr unsigned int descriptorSetLayoutBindingCounts[] = {0};
		constexpr VkDescriptorType descriptorSetLayoutDescriptorTypes[] = { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE };
		constexpr VkShaderStageFlags descriptorLayoutStages[] = { VK_SHADER_STAGE_COMPUTE_BIT };
		constexpr unsigned int descriptorSetlayoutDescriptorCounts[] = { 1 };
		DescriptorSetLayout* pDescriptorSetLayout = pDevice->CreateDescriptorSetLayout(
			descriptorSetLayoutBindingCounts,
			descriptorSetLayoutDescriptorTypes,
			descriptorSetlayoutDescriptorCounts,
			descriptorLayoutStages,
			1
		);

		//Create descriptor set
		DescriptorSet* ppDescriptorSets[requestedSwapchainBufferCount];
		for (unsigned char i = 0; i < requestedSwapchainBufferCount; i++)
		{
			ppDescriptorSets[i] = pDevice->AllocateDescriptorSet(pDescriptorPool,pDescriptorSetLayout);
			pDevice->UpdateDescriptorSetTextureSampler(ppDescriptorSets[i], 0, 0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, ppSwapchainTextureViews[i], nullptr, VK_IMAGE_LAYOUT_GENERAL);
		}

		//Create compute pipeline
		ComputePipeline* pComputePipeline = pDevice->CreateComputePipeline(
			VkPipelineCreateFlags(),
			pComputeShader,
			(const DescriptorSetLayout**)&pDescriptorSetLayout, 1,
			nullptr, 0);

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
		pDevice->SubmitCommandLists(pDefaultQueue, (const CommandList**) & pCmdList, 1, nullptr, pCmdFence, nullptr, 0, nullptr, 0);
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

			//Compute barrier
			pCmdList->SetPipelineTextureBarrier(
				pSwapchain->GetTexture(swapchainImageIndex),
				VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_GENERAL,
				VK_QUEUE_GRAPHICS_BIT, VK_QUEUE_GRAPHICS_BIT,
				VK_IMAGE_ASPECT_COLOR_BIT, 0, 0,
				VK_ACCESS_COLOR_ATTACHMENT_READ_BIT, VK_ACCESS_SHADER_WRITE_BIT,
				VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
				VkDependencyFlags()
			);

			//Set compute pipeline
			pCmdList->SetPipeline(pComputePipeline);

			//Set descriptor heaps
			pCmdList->SetDescriptorSets((const DescriptorSet**)&ppDescriptorSets[swapchainImageIndex], 1, 0, nullptr, 0);

			//Dispatch compute
			pCmdList->DispatchCompute(pWindow->GetWidth()/10, pWindow->GetHeight() / 10, 1);

			//Attachment read barrier
			pCmdList->SetPipelineTextureBarrier(
				pSwapchain->GetTexture(swapchainImageIndex),
				VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				VK_QUEUE_GRAPHICS_BIT, VK_QUEUE_GRAPHICS_BIT,
				VK_IMAGE_ASPECT_COLOR_BIT, 0, 0,
				VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
				VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				VkDependencyFlags());

			//Set dynamic rendering color attachment
			VkClearColorValue colorAttachmentClearValue = {};
			
			//Add dynamic rendering color attachment
			pCmdList->AddDynamicRenderingColorAttachment(
				ppSwapchainTextureViews[swapchainImageIndex],
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				VkResolveModeFlags(),
				nullptr,
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				VK_ATTACHMENT_LOAD_OP_LOAD,
				VK_ATTACHMENT_STORE_OP_STORE,
				colorAttachmentClearValue);

			//Start dynamic rendering
			pCmdList->BeginDynamicRendering(0, 1, 0, 0, pWindow->GetWidth(), pWindow->GetHeight());

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
			pDevice->SubmitCommandLists(pDefaultQueue, (const CommandList**)&pCmdList, 1, nullptr, pCmdFence, nullptr, 0, nullptr, 0);
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
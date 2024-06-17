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

#include <glm/vec3.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <Runtime/Mesh/MeshLoader.h>

namespace Oksijen
{
	struct Vertex
	{
		float Pos[3];
		float Normal[3];
		float Tangent[3];
		float BiTangent[3];
		float Uv[2];
	};
	struct VertexTransformationBuffer
	{
		glm::mat4x4 Mvp;
		glm::mat4x4 Model;
	};
	struct FragmentCameraBuffer
	{
		float ViewPos[3];
	};
	struct MaterialData
	{
		Texture* pColorTexture;
		Texture* pNormalTexture;
		Texture* pRoughnessTexture;
		DescriptorSet* pDescriptorSet;
		Pipeline* pPipeline;
	};
	struct RenderableObject
	{
		glm::vec3 Position;
		glm::vec3 Scale;
		glm::vec3 Rotation;
		GraphicsBuffer* pVertexBuffer;
		GraphicsBuffer* pIndexBuffer;
		GraphicsBuffer* pVertexTransformBuffer;
		GraphicsBuffer* pFragmentCameraBuffer;
		GraphicsBuffer* pStageBuffer;
	};
	struct TextureLoadData
	{
		unsigned char* pData;
		unsigned long long DataSize;
		unsigned int Width;
		unsigned int Height;
		unsigned char Channels;
	};

	static const char vertexShaderSource[] =
		R"STR(
			#version 430 core

			layout(location = 0) in vec3 aPos;
			layout(location = 1) in vec3 aNormal;
			layout(location = 2) in vec3 aTangent;
			layout(location = 3) in vec3 aBiTangent;
			layout(location = 4) in vec2 aUv;

			layout(location = 0) out vec2 f_Uv;
			layout(location = 1) out vec3 f_Normal;
			layout(location = 2) out vec3 f_WorldPos;
			layout(location = 3) out mat3 f_TBN;

			layout(std140,binding = 0,set = 0) uniform VertexTransformBuffer
			{
				mat4 Mvp;
				mat4 Model;
			};
			
			
			void main()
			{
				gl_Position = vec4(aPos,1.0f)*Mvp;

				vec3 T = normalize(vec3(Model * vec4(aTangent,   0.0)));
				vec3 B = normalize(vec3(Model * vec4(aBiTangent, 0.0)));
				vec3 N = normalize(vec3(Model * vec4(aNormal,    0.0)));
				
				f_Normal = aNormal;
				f_TBN = mat3(T,B,N);
				f_Uv = aUv;
				f_WorldPos = (Model*vec4(aPos,1.0f)).xyz;
			}
)STR";

	static const char pixelShaderSource[] =
		R"STR(
			#version 430 core
			layout(location = 0) in vec2 f_Uv;
			layout(location = 1) in vec3 f_Normal;
			layout(location = 2) in vec3 f_WorldPos;
			layout(location = 3) in mat3 f_TBN;

			layout(location = 0) out vec4 ColorOut;
			

			layout(std140,binding = 1,set = 0) uniform FragmentCameraBuffer
			{
				vec3 ViewPos;
			};

			layout(binding = 2,set = 0) uniform texture2D albedoTexture;
			layout(binding = 3,set = 0) uniform texture2D normalTexture;
			layout(binding = 4,set = 0) uniform texture2D roughnessTexture;
			layout(binding = 5,set = 0) uniform sampler sampler0;

			const vec4 c_LightColor = vec4(0.8f,0.8f,0.8f,1.0f);
			const float c_Metallic = 0.1f;
			const float PI = 3.14159265359;
			
			vec3 FresnelSchlick(float cosTheta,vec3 f0)
			{
				return f0 + (1.0f-f0) * pow(clamp(1.0f-cosTheta,0.0f,1.0f),5.0f);
			}
			float DistributionGGX(vec3 n,vec3 h,float roughness)
			{
				float a = roughness*roughness;
				float a2 = a*a;
				float ndotH = max(dot(n,h),0.0f);
				float ndotH2 = ndotH*ndotH;

				float num = a2;
				float denom = (ndotH2*(a2-1.0f) + 1.0f);
				denom = PI*denom*denom;
			
				return num / denom;
			}
			float GeometrySchlickGGX(float ndotV,float roughness)
			{
				float r = (roughness + 1.0f);
				float k = (r*r) / 8.0f;
				
				float num = ndotV;
				float denom = ndotV * (1.0f-k) + k;

				return num / denom;
			}
			float GeometrySmith(vec3 n,vec3 v,vec3 l,float roughness)
			{
				float ndotV = max(dot(n,v),0.0f);
				float ndotL = max(dot(n,l),0.0f);
				float ggx1 = GeometrySchlickGGX(ndotL,roughness);
				float ggx2 = GeometrySchlickGGX(ndotV,roughness);

				return ggx1*ggx2;
			}
			void main()
			{
				vec3 albedo = texture(sampler2D(albedoTexture,sampler0),f_Uv).rgb;
				vec3 roughness = texture(sampler2D(roughnessTexture,sampler0),f_Uv).rgb;
				float ao = 0.5f;
				
				vec3 n = texture(sampler2D(normalTexture,sampler0),f_Uv).rgb;
				n = n*2.0f-1.0f;
				n = normalize(f_TBN*n);
				
				vec3 f0 = vec3(0.04f);
				f0 = mix(f0,albedo,c_Metallic);

				vec3 lo = vec3(0.0f);
				vec3 lightPos = vec3(0,-3,0);
				float lightPower = 0.05f;
				vec3 v = normalize(ViewPos-f_WorldPos);
				vec3 l = normalize(lightPos-f_WorldPos);
				vec3 h = normalize(v + l);
				float distance = length(lightPos-f_WorldPos);
				float attenuation = 1.0f / (distance*distance);
				vec3 radiance = (c_LightColor*attenuation*lightPower).rgb;

				float ndf = DistributionGGX(n,h,roughness.r);
				float g = GeometrySmith(n,v,l,roughness.r);
				vec3 f = FresnelSchlick(max(dot(h,v),0.0f),f0);

				vec3 ks = f;
				vec3 kd = vec3(1.0f) - ks;
				kd *= 1.0f-c_Metallic;
	
				vec3 numerator = ndf*g*f;
				float denominator = 4.0f*max(dot(n,v),0.0f) * max(dot(n,l),0.0f) * 0.0001f;
				vec3 specular = numerator / denominator;


				float ndotl = max(dot(n,l),0.0f);
				lo +=(kd*albedo/PI + specular)*radiance*ndotl;
				
				vec3 ambient = vec3(0.03f)*albedo*ao;
				vec3 color = ambient + lo;
				color = color / (color + vec3(1.0f));
				color = pow(color,vec3(1.0f / 2.2f));
			
				ColorOut = vec4(color,1.0f);
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
		windowDesc.Title = "11_SponzaImport";

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
		GraphicsQueue* pDefaultGraphicsQueue = pDevice->RentQueue(VK_QUEUE_GRAPHICS_BIT);

		//Compile shaders
		unsigned char* pVertexShaderSPIRVBytes = nullptr;
		unsigned int vertexShaderSPIRVByteCount = 0;
		std::string vertexShaderCompilationErrors;
		DEV_ASSERT(ShaderCompiler::TextToSPIRV(vertexShaderSource, "main", shaderc_vertex_shader, shaderc_source_language_glsl, &pVertexShaderSPIRVBytes, vertexShaderSPIRVByteCount, vertexShaderCompilationErrors),"Main","Failed to compile vertex shader with logs: %s",vertexShaderCompilationErrors.c_str());

		unsigned char* pFragmentShaderSPIRVBytes = nullptr;
		unsigned int fragmentShaderSPIRVByteCount = 0;
		std::string fragmentShaderCompilationErrors;
		DEV_ASSERT(ShaderCompiler::TextToSPIRV(pixelShaderSource, "main", shaderc_fragment_shader, shaderc_source_language_glsl, &pFragmentShaderSPIRVBytes, fragmentShaderSPIRVByteCount, fragmentShaderCompilationErrors), "Main", "Failed to compile the fragment shader with logs: %s",fragmentShaderCompilationErrors.c_str());

		//Create shaders
		Shader* pVertexShader = pDevice->CreateShader("main",VK_SHADER_STAGE_VERTEX_BIT,pVertexShaderSPIRVBytes, vertexShaderSPIRVByteCount);
		Shader* pFragmentShader = pDevice->CreateShader("main",VK_SHADER_STAGE_FRAGMENT_BIT, pFragmentShaderSPIRVBytes, fragmentShaderSPIRVByteCount);

		//Create command pool
		CommandPool* pCmdPool = pDevice->CreateCommandPool(VK_QUEUE_GRAPHICS_BIT);

		//Create command list
		CommandList* pCmdList = pDevice->AllocateCommandList(pCmdPool);

		//Create general use-case fence
		Fence* pCmdFence = pDevice->CreateFence(false);

		//Load sponza mesh
		std::string sponzaMeshPath = RES_PATH;
		sponzaMeshPath += "/Sponza.fbx";
		std::vector<MeshImportData> sponzaMeshes;
		std::vector<MaterialImportData> sponzeMaterials;
		MeshLoader::LoadMesh(sponzaMeshPath, aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_CalcTangentSpace | aiProcess_FlipUVs, sponzaMeshes, sponzeMaterials);

		//Get memory requirement for the sponza mesh
		unsigned long long applicationMemoryRequirement = 0;
		for (const MeshImportData& mesh : sponzaMeshes)
		{
			applicationMemoryRequirement += (mesh.VertexCount * sizeof(Vertex)) + (mesh.Triangles.size() * sizeof(unsigned int));
		}

		//Load sponza textures
		std::vector<TextureLoadData> colorTextures;
		std::vector<TextureLoadData> normalTextures;
		std::vector<TextureLoadData> roughnessTextures;

		//Load sponza color textures
		for (const MaterialImportData& importedMaterial : sponzeMaterials)
		{
			TextureLoadData loadData = {};

			std::string path = RES_PATH;
			path += "/";
			path += importedMaterial.BaseColorTextureName;
			DEV_ASSERT(TextureLoader::LoadFromPath(path, 4, &loadData.pData, loadData.DataSize, loadData.Width, loadData.Height, loadData.Channels), "Main", "Failed to load the color texture");

			colorTextures.push_back(loadData);

			applicationMemoryRequirement += loadData.DataSize;
		}

		//Load sponza normal textures
		const char* pNormalTexturePaths[] =
		{
			"",
			""
		};
		for (const const char* pPath : nullptr)
		{

		}

		applicationMemoryRequirement += pWindow->GetWidth() * pWindow->GetHeight() * 4; // Depth texture

		//Allocate memory
		GraphicsMemory* pHostMemory = pDevice->AllocateMemory(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, applicationMemoryRequirement);
		GraphicsMemory* pDeviceMemory = pDevice->AllocateMemory(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, applicationMemoryRequirement);

		//Create descriptor pool
		//DescriptorPool* pDescriptorPool = pDevice->CreateDescriptorPool(VkDescriptorPoolCreateFlags(),sponzaMeshes,nullptr,nullptr,1);

		//Create surface
		Surface* pSurface = pInstance->CreateSurface(pWindow);

		//Test surface
		const unsigned int requestedSwapchainBufferCount = 2;
		const VkFormat requestedSwapchainFormat = VK_FORMAT_R8G8B8A8_UNORM;
		const VkImageUsageFlags requestedSwapchainImageUsageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		const VkColorSpaceKHR requestedSwapchainFormatColorspace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
		const VkPresentModeKHR requestedSwapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;
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
		Swapchain* pSwapchain = pDevice->CreateSwapchain(pSurface, pDefaultGraphicsQueue, requestedSwapchainBufferCount, requestedSwapchainWidth, requestedSwapchainHeight,1, requestedSwapchainPresentMode, requestedSwapchainFormat, requestedSwapchainFormatColorspace, requestedSwapchainImageUsageFlags);

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

		//Set pipeline barriers for swapchain the images
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

		//Application loop
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
			VkClearColorValue colorAttachmentClearValue = {};
			if (bRed)
			{
				colorAttachmentClearValue.float32[0] = 1.0f;
				colorAttachmentClearValue.float32[1] = 0.0f;
				colorAttachmentClearValue.float32[2] = 0.0f;
				colorAttachmentClearValue.float32[3] = 1.0f;
			}
			else
			{
				colorAttachmentClearValue.float32[0] = 0.0f;
				colorAttachmentClearValue.float32[1] = 0.0f;
				colorAttachmentClearValue.float32[2] = 1.0f;
				colorAttachmentClearValue.float32[3] = 1.0f;
			}

			pCmdList->AddDynamicRenderingColorAttachment(
				ppSwapchainTextureViews[swapchainImageIndex],
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				VkResolveModeFlags(),
				nullptr,
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				VK_ATTACHMENT_LOAD_OP_CLEAR,
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
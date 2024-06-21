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
	struct RenderableObject
	{
		glm::vec3 Position = {0,0,0};
		glm::vec3 Scale = { 1,1,1 };
		glm::vec3 Rotation = { 0,0,0 };
		GraphicsBuffer* pVertexBuffer = nullptr;
		GraphicsBuffer* pIndexBuffer = nullptr;
		GraphicsBuffer* pTransformBuffer = nullptr;
		GraphicsBuffer* pTransformStageBuffer = nullptr;
		DescriptorSet* pDescriptorSet = nullptr;
	};
	struct MaterialData
	{
		Texture* pColorTexture = nullptr;
		Texture* pNormalTexture = nullptr;
		Texture* pRoughnessTexture = nullptr;
		TextureView* pColorTextureView = nullptr;
		TextureView* pNormalTextureView = nullptr;
		TextureView* pRoughnessTextureView = nullptr;
		DescriptorSet* pDescriptorSet = nullptr;

		std::vector<RenderableObject> Renderables;
	};
	struct TextureLoadData
	{
		unsigned char* pData = nullptr;
		unsigned long long DataSize = 0;
		unsigned int Width = 0;
		unsigned int Height = 0;
		unsigned char Channels = 0;

		void Free()
		{
			if (pData == nullptr)
				return;

			delete pData;
			pData = nullptr;
			DataSize = 0;
			Width = 0;
			Height = 0;
			Channels = 0;
		}
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
			

			layout(binding = 0,set = 1) uniform texture2D albedoTexture;
			layout(binding = 1,set = 1) uniform texture2D normalTexture;
			layout(binding = 2,set = 1) uniform texture2D roughnessTexture;

			layout(std140,binding = 0,set = 2) uniform FragmentCameraBuffer
			{
				vec3 ViewPos;
			};
			layout(binding = 1,set = 2) uniform sampler sampler0;

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
				vec3 lightPos = vec3(0,0,0);
				float lightPower = 0.05f;
				vec3 v = normalize(ViewPos-f_WorldPos);
				vec3 l = normalize(lightPos-f_WorldPos);
				vec3 h = normalize(v + l);
				float distance = length(lightPos-f_WorldPos);
				float attenuation = 1.0f / (distance*distance);
				vec3 radiance = (c_LightColor*attenuation*lightPower).rgb;

				float ndf = DistributionGGX(n,h,roughness.g);
				float g = GeometrySmith(n,v,l,roughness.g);
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
				ColorOut = vec4(albedo,1.0f)*dot(n,vec3(1,1,1));
				ColorOut = vec4(albedo,1.0f);
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
		Swapchain* pSwapchain = pDevice->CreateSwapchain(pSurface, pDefaultGraphicsQueue, requestedSwapchainBufferCount, requestedSwapchainWidth, requestedSwapchainHeight, 1, requestedSwapchainPresentMode, requestedSwapchainFormat, requestedSwapchainFormatColorspace, requestedSwapchainImageUsageFlags);

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

		//Set pipeline barriers for swapchain the images
		pCmdList->Begin();
		for (unsigned char i = 0; i < requestedSwapchainBufferCount; i++)
		{
			pCmdList->SetPipelineTextureBarrier(
				pSwapchain->GetTexture(i),
				VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
				VK_QUEUE_GRAPHICS_BIT, VK_QUEUE_GRAPHICS_BIT,
				VK_IMAGE_ASPECT_COLOR_BIT, 0, 0,
				VK_ACCESS_NONE, VK_ACCESS_COLOR_ATTACHMENT_READ_BIT,
				VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				VkDependencyFlags());
		}
		pCmdList->End();
		pDevice->SubmitCommandLists(pDefaultGraphicsQueue, (const CommandList**)&pCmdList, 1, nullptr, pCmdFence, nullptr, 0, nullptr, 0);
		pCmdFence->Wait();
		pCmdFence->Reset();

		//Load sponza mesh
		std::string sponzaMeshPath = RES_PATH;
		sponzaMeshPath += "/Sponza/Sponza.gltf";
		std::vector<MeshImportData> sponzaMeshes;
		std::vector<MaterialImportData> sponzeMaterials;
		MeshLoader::LoadMesh(sponzaMeshPath, aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_CalcTangentSpace | aiProcess_FlipUVs, sponzaMeshes, sponzeMaterials);

		//Get memory requirement for the sponza mesh
		unsigned long long applicationMemoryRequirement = 0;
		for (const MeshImportData& mesh : sponzaMeshes)
		{
			applicationMemoryRequirement += (mesh.VertexCount * sizeof(Vertex)) + (mesh.Triangles.size() * sizeof(unsigned int));
			applicationMemoryRequirement += sizeof(VertexTransformationBuffer);
		}

		//Load sponza textures
		std::vector<TextureLoadData> colorTextures;
		std::vector<TextureLoadData> normalTextures;
		std::vector<TextureLoadData> roughnessTextures;

		//Load sponza textures
		for (const MaterialImportData& importedMaterial : sponzeMaterials)
		{
			//Load color
			if (importedMaterial.BaseColorTextureName != "")
			{
				TextureLoadData loadData = {};

				std::string path = RES_PATH;
				path += "/Sponza/";
				path += importedMaterial.BaseColorTextureName;
				DEV_ASSERT(TextureLoader::LoadFromPath(path, 4, &loadData.pData, loadData.DataSize, loadData.Width, loadData.Height, loadData.Channels), "Main", "Failed to load the color texture");

				colorTextures.push_back(loadData);

				applicationMemoryRequirement += loadData.DataSize;
			}
			else
			{
				colorTextures.push_back({});
			}

			//Load normal
			if (importedMaterial.NormalTextureName != "")
			{
				TextureLoadData loadData = {};

				std::string path = RES_PATH;
				path += "/Sponza/";
				path += importedMaterial.NormalTextureName;
				DEV_ASSERT(TextureLoader::LoadFromPath(path, 4, &loadData.pData, loadData.DataSize, loadData.Width, loadData.Height, loadData.Channels), "Main", "Failed to load the normal texture");

				normalTextures.push_back(loadData);

				applicationMemoryRequirement += loadData.DataSize;
			}
			else
			{
				normalTextures.push_back({});
			}

			//Load roughness
			if (importedMaterial.RoughnessTextureName != "")
			{
				TextureLoadData loadData = {};

				std::string path = RES_PATH;
				path += "/Sponza/";
				path += importedMaterial.RoughnessTextureName;
				DEV_ASSERT(TextureLoader::LoadFromPath(path, 4, &loadData.pData, loadData.DataSize, loadData.Width, loadData.Height, loadData.Channels), "Main", "Failed to load the roughness texture");

				roughnessTextures.push_back(loadData);

				applicationMemoryRequirement += loadData.DataSize;
			}
			else
			{
				roughnessTextures.push_back({});
			}

		}

		applicationMemoryRequirement += pWindow->GetWidth() * pWindow->GetHeight() * 4; // Depth texture
		applicationMemoryRequirement += sizeof(FragmentCameraBuffer); // 1 fragment camera buffer
		applicationMemoryRequirement += MB_TO_BYTE(10); // manual offset

		//Allocate memory
		GraphicsMemory* pHostMemory = pDevice->AllocateMemory(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, applicationMemoryRequirement);
		GraphicsMemory* pDeviceMemory = pDevice->AllocateMemory(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, applicationMemoryRequirement);

		//Create static descriptor layout
		const unsigned int staticSetLayoutBindings[] = { 0,1};
		const VkDescriptorType staticSetLayoutDescriptorTypes[] = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,VK_DESCRIPTOR_TYPE_SAMPLER };
		const unsigned int staticSetLayoutDescriptorCounts[] = { 1,1};
		const VkShaderStageFlags staticSetLayoutShaderStages[] = { VK_SHADER_STAGE_FRAGMENT_BIT,VK_SHADER_STAGE_FRAGMENT_BIT };

		DescriptorSetLayout* pStaticSetLayout = pDevice->CreateDescriptorSetLayout(staticSetLayoutBindings, staticSetLayoutDescriptorTypes, staticSetLayoutDescriptorCounts, staticSetLayoutShaderStages, 2);

		//Create per material descriptor layout
		const unsigned int materialSetLayoutBindings[] = { 0,1,2 };
		const VkDescriptorType materialSetLayoutDescriptorTypes[] = { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE };
		const unsigned int materialSetLayoutDescriptorCounts[] = { 1,1,1,};
		const VkShaderStageFlags materialSetLayoutShaderStages[] = { VK_SHADER_STAGE_FRAGMENT_BIT,VK_SHADER_STAGE_FRAGMENT_BIT,VK_SHADER_STAGE_FRAGMENT_BIT };

		DescriptorSetLayout* pMaterialSetLayout = pDevice->CreateDescriptorSetLayout(materialSetLayoutBindings, materialSetLayoutDescriptorTypes, materialSetLayoutDescriptorCounts, materialSetLayoutShaderStages, 3);

		//Create per renderable descriptor layout
		const unsigned int renderableSetLayoutBindings[] = { 0 };
		const VkDescriptorType renderableSetLayoutDescriptorTypes[] = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER };
		const unsigned int renderableSetLayoutDescriptorCounts[] = { 1 };
		const VkShaderStageFlags renderableSetLayoutShaderStages[] = { VK_SHADER_STAGE_VERTEX_BIT };

		DescriptorSetLayout* pRenderableSetLayout = pDevice->CreateDescriptorSetLayout(renderableSetLayoutBindings, renderableSetLayoutDescriptorTypes, renderableSetLayoutDescriptorCounts, renderableSetLayoutShaderStages, 1);

		//Create descriptor pool
		const unsigned int poolDescriptorCounts[] = { sponzaMeshes.size() + 1 ,sponzeMaterials.size()*3 ,1 };
		const VkDescriptorType poolDescriptorTypes[] = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,VK_DESCRIPTOR_TYPE_SAMPLER };
		DescriptorPool* pDescriptorPool = pDevice->CreateDescriptorPool(VkDescriptorPoolCreateFlags(), sponzeMaterials.size() + sponzaMeshes.size() + 1, poolDescriptorTypes, poolDescriptorCounts, 3);

		//Create default resources
		Texture* pDefaultTexture = pDevice->CreateTexture(pDeviceMemory, VK_IMAGE_TYPE_2D, VK_FORMAT_R8G8B8A8_UNORM, 1, 1, 1, 1, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT, VK_SHARING_MODE_EXCLUSIVE, VK_IMAGE_LAYOUT_UNDEFINED);
		Sampler* pDefaultSampler = pDevice->CreateSampler(VK_FILTER_LINEAR, VK_FILTER_NEAREST, VK_SAMPLER_MIPMAP_MODE_NEAREST, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT, 0, false, 0, VK_FALSE, VK_COMPARE_OP_NEVER, 0, 0, VK_BORDER_COLOR_INT_OPAQUE_WHITE, VK_FALSE);
		GraphicsBuffer* pCameraViewBuffer = pDevice->CreateBuffer(pDeviceMemory, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE, sizeof(FragmentCameraBuffer));
		GraphicsBuffer* pCameraViewStageBuffer = pDevice->CreateBuffer(pHostMemory, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_EXCLUSIVE, sizeof(FragmentCameraBuffer));

		constexpr VkComponentMapping mapping =
		{
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY
		};
		TextureView* pDefaultTextureView = pDevice->CreateTextureView(pDefaultTexture, 0, 0, VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_R8G8B8A8_UNORM, mapping, VK_IMAGE_ASPECT_COLOR_BIT);

		//Create static descriptor set
		DescriptorSet* pStaticDescriptorSet = pDevice->AllocateDescriptorSet(pDescriptorPool,pStaticSetLayout);

		//Update static descriptor set
		pDevice->UpdateDescriptorSetBuffer(pStaticDescriptorSet, 0, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, pCameraViewBuffer, 0, pCameraViewBuffer->GetSize());
		pDevice->UpdateDescriptorSetTextureSampler(pStaticDescriptorSet, 1, 0, VK_DESCRIPTOR_TYPE_SAMPLER, nullptr, pDefaultSampler, VK_IMAGE_LAYOUT_UNDEFINED);

		//Load materials
		std::vector<MaterialData> materials;
		pCmdList->Begin();
		pCmdList->SetPipelineTextureBarrier(pDefaultTexture, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_QUEUE_GRAPHICS_BIT, VK_QUEUE_GRAPHICS_BIT, VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VkDependencyFlags());
		std::vector<GraphicsBuffer*> textureTempBuffers;
		for (unsigned int i = 0;i<sponzeMaterials.size();i++)
		{
			//Get texture load data
			const TextureLoadData& colorTextureLoadData = colorTextures[i];
			const TextureLoadData& normalTextureLoadData = normalTextures[i];
			const TextureLoadData& roughnessTextureLoadaData = roughnessTextures[i];

			//Create new material
			MaterialData material = {};

			//Create color texture
			if (colorTextureLoadData.pData != nullptr)
			{
				material.pColorTexture = pDevice->CreateTexture(pDeviceMemory, VK_IMAGE_TYPE_2D, VK_FORMAT_R8G8B8A8_UNORM, colorTextureLoadData.Width, colorTextureLoadData.Height, 1, 1, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_SHARING_MODE_EXCLUSIVE, VK_IMAGE_LAYOUT_UNDEFINED);
				material.pColorTextureView = pDevice->CreateTextureView(material.pColorTexture, 0, 0, VK_IMAGE_VIEW_TYPE_2D, material.pColorTexture->GetFormat(), mapping, VK_IMAGE_ASPECT_COLOR_BIT);

				GraphicsBuffer* pStageBuffer = pDevice->CreateBuffer(pHostMemory, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_EXCLUSIVE, colorTextureLoadData.DataSize);
				pDevice->UpdateHostBuffer(pStageBuffer, colorTextureLoadData.pData, colorTextureLoadData.DataSize, 0);

				pCmdList->SetPipelineTextureBarrier(material.pColorTexture, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_QUEUE_GRAPHICS_BIT, VK_QUEUE_GRAPHICS_BIT, VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, VK_ACCESS_NONE, VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VkDependencyFlags());
				pCmdList->CopyBufferTexture(pStageBuffer, material.pColorTexture, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 0, 0, 0, VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1, 0, 0, 0, material.pColorTexture->GetWidth(), material.pColorTexture->GetHeight(), 1);
				pCmdList->SetPipelineTextureBarrier(material.pColorTexture, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_QUEUE_GRAPHICS_BIT, VK_QUEUE_GRAPHICS_BIT, VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VkDependencyFlags());

				textureTempBuffers.push_back(pStageBuffer);
			}
			else
			{
				material.pColorTexture = pDefaultTexture;
				material.pColorTextureView = pDefaultTextureView;
			}
			
			//Create normal texture
			if (normalTextureLoadData.pData != nullptr)
			{
				material.pNormalTexture = pDevice->CreateTexture(pDeviceMemory, VK_IMAGE_TYPE_2D, VK_FORMAT_R8G8B8A8_UNORM, normalTextureLoadData.Width, normalTextureLoadData.Height, 1, 1, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_SHARING_MODE_EXCLUSIVE, VK_IMAGE_LAYOUT_UNDEFINED);
				material.pNormalTextureView = pDevice->CreateTextureView(material.pNormalTexture, 0, 0, VK_IMAGE_VIEW_TYPE_2D, material.pNormalTexture->GetFormat(), mapping, VK_IMAGE_ASPECT_COLOR_BIT);

				GraphicsBuffer* pStageBuffer = pDevice->CreateBuffer(pHostMemory, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_EXCLUSIVE, normalTextureLoadData.DataSize);
				pDevice->UpdateHostBuffer(pStageBuffer, normalTextureLoadData.pData, normalTextureLoadData.DataSize, 0);

				pCmdList->SetPipelineTextureBarrier(material.pNormalTexture, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_QUEUE_GRAPHICS_BIT, VK_QUEUE_GRAPHICS_BIT, VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, VK_ACCESS_NONE, VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VkDependencyFlags());
				pCmdList->CopyBufferTexture(pStageBuffer, material.pNormalTexture, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 0, 0, 0, VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1, 0, 0, 0, material.pNormalTexture->GetWidth(), material.pNormalTexture->GetHeight(), 1);
				pCmdList->SetPipelineTextureBarrier(material.pNormalTexture, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_QUEUE_GRAPHICS_BIT, VK_QUEUE_GRAPHICS_BIT, VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VkDependencyFlags());

				textureTempBuffers.push_back(pStageBuffer);
			}
			else
			{
				material.pNormalTexture = pDefaultTexture;
				material.pNormalTextureView = pDefaultTextureView;
			}

			//Create roughness texture
			if (roughnessTextureLoadaData.pData != nullptr)
			{
				material.pRoughnessTexture = pDevice->CreateTexture(pDeviceMemory, VK_IMAGE_TYPE_2D, VK_FORMAT_R8G8B8A8_UNORM, roughnessTextureLoadaData.Width, roughnessTextureLoadaData.Height, 1, 1, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_SHARING_MODE_EXCLUSIVE, VK_IMAGE_LAYOUT_UNDEFINED);
				material.pRoughnessTextureView = pDevice->CreateTextureView(material.pRoughnessTexture, 0, 0, VK_IMAGE_VIEW_TYPE_2D, material.pRoughnessTexture->GetFormat(), mapping, VK_IMAGE_ASPECT_COLOR_BIT);

				GraphicsBuffer* pStageBuffer = pDevice->CreateBuffer(pHostMemory, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_EXCLUSIVE, roughnessTextureLoadaData.DataSize);
				pDevice->UpdateHostBuffer(pStageBuffer, roughnessTextureLoadaData.pData, roughnessTextureLoadaData.DataSize, 0);

				pCmdList->SetPipelineTextureBarrier(material.pRoughnessTexture, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_QUEUE_GRAPHICS_BIT, VK_QUEUE_GRAPHICS_BIT, VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, VK_ACCESS_NONE, VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VkDependencyFlags());
				pCmdList->CopyBufferTexture(pStageBuffer, material.pRoughnessTexture, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 0, 0, 0, VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1, 0, 0, 0, material.pRoughnessTexture->GetWidth(), material.pRoughnessTexture->GetHeight(), 1);
				pCmdList->SetPipelineTextureBarrier(material.pRoughnessTexture, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_QUEUE_GRAPHICS_BIT, VK_QUEUE_GRAPHICS_BIT, VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VkDependencyFlags());

				textureTempBuffers.push_back(pStageBuffer);
			}
			else
			{
				material.pRoughnessTexture = pDefaultTexture;
				material.pRoughnessTextureView = pDefaultTextureView;
			}

			//Create descriptor set
			material.pDescriptorSet = pDevice->AllocateDescriptorSet(pDescriptorPool, pMaterialSetLayout);

			//Update descriptor set
			pDevice->UpdateDescriptorSetTextureSampler(material.pDescriptorSet, 0, 0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, material.pColorTextureView, nullptr, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			pDevice->UpdateDescriptorSetTextureSampler(material.pDescriptorSet, 1, 0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, material.pNormalTextureView, nullptr, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			pDevice->UpdateDescriptorSetTextureSampler(material.pDescriptorSet, 2, 0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, material.pRoughnessTextureView, nullptr, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

			materials.push_back(material);
		}
		pCmdList->End();
		pDevice->SubmitCommandLists(pDefaultGraphicsQueue, (const CommandList**)&pCmdList, 1, nullptr, pCmdFence, nullptr, 0, nullptr, 0);
		pCmdFence->Wait();
		pCmdFence->Reset();

		//Clean texture data
		for (TextureLoadData& data : colorTextures)
		{
			data.Free();
		}
		for (TextureLoadData& data : normalTextures)
		{
			data.Free();
		}
		for (TextureLoadData& data : roughnessTextures)
		{
			data.Free();
		}

		//Load renderable objects
		std::vector<RenderableObject> renderables;
		std::vector<Vertex> cpuVertexBufferTemp;
		cpuVertexBufferTemp.reserve(10000);

		for (const MeshImportData& meshImportData : sponzaMeshes)
		{
			const unsigned long long vertexBufferSize = sizeof(Vertex) * meshImportData.VertexCount;
			const unsigned long long indexBufferSize = sizeof(unsigned int) * meshImportData.Triangles.size();

			//Get mesh material
			MaterialData& materialData = materials[meshImportData.MaterialGroupIndex];

			//Create empty renderable object
			RenderableObject renderable = {};

			//Allocate stage buffers
			GraphicsBuffer* pVertexStageBuffer = pDevice->CreateBuffer(pHostMemory, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_EXCLUSIVE, vertexBufferSize);
			GraphicsBuffer* pIndexStageBuffer = pDevice->CreateBuffer(pHostMemory, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_EXCLUSIVE, indexBufferSize);
			renderable.pTransformStageBuffer = pDevice->CreateBuffer(pHostMemory, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_EXCLUSIVE, sizeof(VertexTransformationBuffer));

			//Allocate device buffers
			renderable.pVertexBuffer = pDevice->CreateBuffer(pDeviceMemory, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_SHARING_MODE_EXCLUSIVE, vertexBufferSize);
			renderable.pIndexBuffer = pDevice->CreateBuffer(pDeviceMemory, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_SHARING_MODE_EXCLUSIVE, indexBufferSize);
			renderable.pTransformBuffer = pDevice->CreateBuffer(pDeviceMemory, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE, sizeof(VertexTransformationBuffer));

			//Update stage buffers
			for (unsigned int i = 0; i < meshImportData.VertexCount; i++)
			{
				const VertexVector3D& pos = meshImportData.Positions[i];
				const VertexVector3D& normal = meshImportData.Normals[i];
				const VertexVector3D& tangent = meshImportData.Tangents[i];
				const VertexVector3D& biTangent = meshImportData.Bitangents[i];
				const VertexVector2D& uv = meshImportData.Uvs[0][i];

				cpuVertexBufferTemp.push_back({ pos.X / 70.0f ,pos.Y / 70.0f ,pos.Z / 70.0f ,normal.X,normal.Y,normal.Z,tangent.X,tangent.Y,tangent.Z,biTangent.X,biTangent.Y,biTangent.Z,uv.X,uv.Y });
			}
			pDevice->UpdateHostBuffer(pVertexStageBuffer, (const unsigned char*)cpuVertexBufferTemp.data(), sizeof(Vertex) * meshImportData.VertexCount, 0);
			pDevice->UpdateHostBuffer(pIndexStageBuffer, (const unsigned char*)meshImportData.Triangles.data(), indexBufferSize, 0);

			//Update vertex&index buffers
			pCmdList->Begin();

			pCmdList->CopyBufferBuffer(pVertexStageBuffer,renderable.pVertexBuffer,0,0, vertexBufferSize);
			pCmdList->CopyBufferBuffer(pIndexStageBuffer, renderable.pIndexBuffer, 0, 0, indexBufferSize);

			pCmdList->End();
			pDevice->SubmitCommandLists(pDefaultGraphicsQueue, (const CommandList**)& pCmdList, 1, nullptr, pCmdFence, nullptr, 0, nullptr, 0);
			pCmdFence->Wait();
			pCmdFence->Reset();

			//Clear cpu temp vertex buffer
			cpuVertexBufferTemp.clear();

			//Delete stage buffers
			delete pVertexStageBuffer;
			delete pIndexStageBuffer;

			//Create descriptor set
			renderable.pDescriptorSet = pDevice->AllocateDescriptorSet(pDescriptorPool, pRenderableSetLayout);

			//Update descriptor set
			pDevice->UpdateDescriptorSetBuffer(renderable.pDescriptorSet, 0, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, renderable.pTransformBuffer, 0, renderable.pTransformBuffer->GetSize());

			//Register renderable
			materialData.Renderables.push_back(renderable);
		}

		for (MeshImportData& data : sponzaMeshes)
		{
			data.Free();
		}

		cpuVertexBufferTemp.clear();
		cpuVertexBufferTemp.shrink_to_fit();

		//Create pipeline
		VkPipelineVertexInputStateCreateInfo vertexInputState = {};
		vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputState.flags = 0;

		VkVertexInputBindingDescription bindingDesc = {};
		bindingDesc.binding = 0;
		bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		bindingDesc.stride = sizeof(Vertex);

		vertexInputState.vertexBindingDescriptionCount = 1;
		vertexInputState.pVertexBindingDescriptions = &bindingDesc;

		VkVertexInputAttributeDescription posAttributeDesc = {};
		posAttributeDesc.binding = 0;
		posAttributeDesc.format = VK_FORMAT_R32G32B32_SFLOAT;
		posAttributeDesc.location = 0;
		posAttributeDesc.offset = 0;

		VkVertexInputAttributeDescription normalAttributeDesc = {};
		normalAttributeDesc.binding = 0;
		normalAttributeDesc.format = VK_FORMAT_R32G32B32_SFLOAT;
		normalAttributeDesc.location = 1;
		normalAttributeDesc.offset = sizeof(float) * 3;

		VkVertexInputAttributeDescription tangentAttributeDesc = {};
		tangentAttributeDesc.binding = 0;
		tangentAttributeDesc.format = VK_FORMAT_R32G32B32_SFLOAT;
		tangentAttributeDesc.location = 2;
		tangentAttributeDesc.offset = sizeof(float) * 6;

		VkVertexInputAttributeDescription biTangentAttributeDesc = {};
		biTangentAttributeDesc.binding = 0;
		biTangentAttributeDesc.format = VK_FORMAT_R32G32B32_SFLOAT;
		biTangentAttributeDesc.location = 3;
		biTangentAttributeDesc.offset = sizeof(float) * 9;

		VkVertexInputAttributeDescription uvAttributeDesc = {};
		uvAttributeDesc.binding = 0;
		uvAttributeDesc.format = VK_FORMAT_R32G32_SFLOAT;
		uvAttributeDesc.location = 4;
		uvAttributeDesc.offset = sizeof(float) * 12;

		VkVertexInputAttributeDescription attributes[] = { posAttributeDesc,normalAttributeDesc,tangentAttributeDesc,biTangentAttributeDesc,uvAttributeDesc };
		vertexInputState.vertexAttributeDescriptionCount = sizeof(attributes) / sizeof(VkVertexInputAttributeDescription);
		vertexInputState.pVertexAttributeDescriptions = attributes;
		vertexInputState.pNext = nullptr;

		VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = {};
		inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssemblyState.primitiveRestartEnable = VK_FALSE;
		inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssemblyState.flags = 0;
		inputAssemblyState.pNext = nullptr;

		VkPipelineRasterizationStateCreateInfo rasterizerDesc = {};
		rasterizerDesc.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizerDesc.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizerDesc.depthBiasClamp = 0;
		rasterizerDesc.depthBiasConstantFactor = 0;
		rasterizerDesc.depthBiasEnable = VK_FALSE;
		rasterizerDesc.depthBiasSlopeFactor = 0;
		rasterizerDesc.depthClampEnable = VK_FALSE;
		rasterizerDesc.flags = 0;
		rasterizerDesc.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterizerDesc.lineWidth = 1.0f;
		rasterizerDesc.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizerDesc.rasterizerDiscardEnable = VK_FALSE;
		rasterizerDesc.pNext = nullptr;

		VkPipelineMultisampleStateCreateInfo multisampleDesc = {};
		multisampleDesc.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampleDesc.alphaToCoverageEnable = VK_FALSE;
		multisampleDesc.alphaToOneEnable = VK_FALSE;
		multisampleDesc.flags = 0;
		multisampleDesc.minSampleShading = 0;
		multisampleDesc.pSampleMask = nullptr;
		multisampleDesc.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampleDesc.sampleShadingEnable = VK_FALSE;
		multisampleDesc.pNext = nullptr;

		VkPipelineDepthStencilStateCreateInfo depthStencilDesc = {};
		depthStencilDesc.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencilDesc.stencilTestEnable = VK_FALSE;
		depthStencilDesc.front = {};
		depthStencilDesc.back = {};
		depthStencilDesc.depthTestEnable = VK_TRUE;
		depthStencilDesc.depthWriteEnable = VK_TRUE;
		depthStencilDesc.depthCompareOp = VK_COMPARE_OP_LESS;
		depthStencilDesc.depthBoundsTestEnable = VK_FALSE;
		depthStencilDesc.maxDepthBounds = 1.0f;
		depthStencilDesc.minDepthBounds = 0.0f;
		depthStencilDesc.pNext = nullptr;

		VkPipelineColorBlendStateCreateInfo blendingDesc = {};
		blendingDesc.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;

		VkPipelineColorBlendAttachmentState colorBlendAttachmentState = {};
		colorBlendAttachmentState.blendEnable = VK_FALSE;
		colorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

		blendingDesc.attachmentCount = 1;
		blendingDesc.pAttachments = &colorBlendAttachmentState;
		blendingDesc.logicOp = VK_LOGIC_OP_CLEAR;
		blendingDesc.logicOpEnable = VK_FALSE;
		blendingDesc.pNext = nullptr;

		const Shader* pPipelineShaders[] = { pVertexShader,pFragmentShader };

		constexpr VkFormat colorAttachmentFormat = VK_FORMAT_R8G8B8A8_UNORM;
		VkPipelineRenderingCreateInfoKHR renderingInfo = {};
		renderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
		renderingInfo.colorAttachmentCount = 1;
		renderingInfo.pColorAttachmentFormats = &colorAttachmentFormat;
		renderingInfo.depthAttachmentFormat = VK_FORMAT_D32_SFLOAT;
		renderingInfo.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;
		renderingInfo.pNext = nullptr;

		VkViewport viewport = {};
		viewport.x = 0;
		viewport.y = 0;
		viewport.width = pWindow->GetWidth();
		viewport.height = pWindow->GetHeight();
		viewport.minDepth = 0;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor = {};
		scissor.offset = { 0,0 };
		scissor.extent = { pWindow->GetWidth(),pWindow->GetHeight() };

		VkPipelineViewportStateCreateInfo viewportDesc = {};
		viewportDesc.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportDesc.scissorCount = 1;
		viewportDesc.viewportCount = 1;
		viewportDesc.pViewports = &viewport;
		viewportDesc.pScissors = &scissor;
		viewportDesc.pNext = nullptr;

		DescriptorSetLayout* pSetLayouts[] = { pRenderableSetLayout,pMaterialSetLayout,pStaticSetLayout };
		Pipeline* pPipeline = pDevice->CreateGraphicsPipeline(
			vertexInputState,
			inputAssemblyState,
			nullptr,
			&viewportDesc,
			rasterizerDesc,
			multisampleDesc,
			depthStencilDesc,
			blendingDesc,
			nullptr,
			pPipelineShaders, 2,
			(const DescriptorSetLayout**)pSetLayouts, 3,
			nullptr,
			nullptr, 0,
			&renderingInfo);


		//Create depth texture
		Texture* pDepthTexture = pDevice->CreateTexture(
			pDeviceMemory,
			VK_IMAGE_TYPE_2D,
			VK_FORMAT_D32_SFLOAT,
			pWindow->GetWidth(), pWindow->GetHeight(), 1,
			1, 1,
			VK_SAMPLE_COUNT_1_BIT,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
			VK_SHARING_MODE_EXCLUSIVE,
			VK_IMAGE_LAYOUT_UNDEFINED);

		TextureView* pDepthTextureView = pDevice->CreateTextureView(
			pDepthTexture,
			0, 0,
			VK_IMAGE_VIEW_TYPE_2D,
			VK_FORMAT_D32_SFLOAT, { VK_COMPONENT_SWIZZLE_IDENTITY,VK_COMPONENT_SWIZZLE_IDENTITY,VK_COMPONENT_SWIZZLE_IDENTITY,VK_COMPONENT_SWIZZLE_IDENTITY },
			VK_IMAGE_ASPECT_DEPTH_BIT);

		//Set depth texture layout
		pCmdList->Begin();
		pCmdList->SetPipelineTextureBarrier(
			pDepthTexture,
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
			VK_QUEUE_GRAPHICS_BIT, VK_QUEUE_GRAPHICS_BIT,
			VK_IMAGE_ASPECT_DEPTH_BIT, 0, 0,
			VK_ACCESS_NONE, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, VkDependencyFlags());

		pCmdList->End();
		pDevice->SubmitCommandLists(pDefaultGraphicsQueue, (const CommandList**)&pCmdList, 1, nullptr, pCmdFence, nullptr, 0, nullptr, 0);
		pCmdFence->Wait();
		pCmdFence->Reset();

		//Create present image fence
		Fence* pPresentImageAcquireFence = pDevice->CreateFence(false);

		//Application loop
		glm::vec3 cameraPosition = { 0,0,-3 };
		glm::vec3 cameraForward = { 0,0,1 };
		const glm::vec3 relativeUp = { 0,-1,0 };
		bool bRed = true;
		while (pWindow->IsActive())
		{
			// poll window events
			pWindow->PollEvents();
			if (!pWindow->IsActive())
			{
				break;
			}

			//FIRST Acquire image index
			const unsigned int swapchainImageIndex = pSwapchain->AcquireImageIndex(pPresentImageAcquireFence, nullptr);
			pPresentImageAcquireFence->Wait();
			pPresentImageAcquireFence->Reset();

			//Camera controls
			const std::vector<WindowEventData>& windowEvents = pWindow->GetBufferedEvents();
			for (const WindowEventData& eventData : windowEvents)
			{
				 const WindowEventType type = eventData.Type;
				 switch (type)
				 {
				 case Oksijen::WindowEventType::WindowClosed:
					 break;
				 case Oksijen::WindowEventType::WindowMoved:
					 break;
				 case Oksijen::WindowEventType::WindowResized:
					 break;
				 case Oksijen::WindowEventType::DragDrop:
					 break;
				 case Oksijen::WindowEventType::KeyboardDown:
				 {
					 if (eventData.KeyboardKey == KeyboardKeys::Q)
					 {
						 glm::mat4x4 rotationMatrix(1);
						 rotationMatrix = glm::rotate(rotationMatrix, glm::radians(-2.0f), glm::vec3(0, 1, 0));
						 cameraForward = glm::vec3(rotationMatrix * glm::vec4(cameraForward, 1.0));
					 }
					 else if (eventData.KeyboardKey == KeyboardKeys::E)
					 {
						 glm::mat4x4 rotationMatrix(1);
						 rotationMatrix = glm::rotate(rotationMatrix, glm::radians(2.0f), glm::vec3(0, 1, 0));
						 cameraForward = glm::vec3(rotationMatrix * glm::vec4(cameraForward, 1.0));
					 }

					 if (eventData.KeyboardKey == KeyboardKeys::W)
					 {
						 cameraPosition += cameraForward;
					 }
					 else if (eventData.KeyboardKey == KeyboardKeys::S)
					 {
						 cameraPosition -= cameraForward;
					 }
					 if (eventData.KeyboardKey == KeyboardKeys::A)
					 {
						 cameraPosition -= glm::cross({0,1,0},cameraForward);
					 }
					 else if (eventData.KeyboardKey == KeyboardKeys::D)
					 {
						 cameraPosition += glm::cross({ 0,1,0 }, cameraForward);
					 }
					 if (eventData.KeyboardKey == KeyboardKeys::Space)
					 {
						 cameraPosition += glm::vec3(0,1,0);
					 }
					 else if (eventData.KeyboardKey == KeyboardKeys::LeftControl)
					 {
						 cameraPosition -= glm::vec3(0, 1, 0);
					 }
					 break;
				 }
				 case Oksijen::WindowEventType::KeyboardUp:
					 break;
				 case Oksijen::WindowEventType::Char:
					 break;
				 case Oksijen::WindowEventType::MouseButtonDown:
					 break;
				 case Oksijen::WindowEventType::MouseButtonUp:
					 break;
				 case Oksijen::WindowEventType::MouseMoved:
					 break;
				 case Oksijen::WindowEventType::MouseScrolled:
					 break;
				 default:
					 break;
				 }
			}
			//Update camera stage buffer
			pDevice->UpdateHostBuffer(pCameraViewStageBuffer, (const unsigned char*)&cameraPosition, sizeof(cameraPosition),0);

			//Update renderable transform stage buffers
			const glm::mat4x4 projection = glm::perspective(glm::radians(60.0f), pWindow->GetWidth() / (float)pWindow->GetHeight(), 0.001f, 100.0f);
			const glm::mat4x4 view = glm::lookAt(cameraPosition,cameraPosition + cameraForward, relativeUp);

			for (const MaterialData& material : materials)
			{
				for (const RenderableObject& renderable : material.Renderables)
				{
					glm::mat4x4 model(1);
					model = glm::translate(model, renderable.Position);
					model = glm::scale(model, renderable.Scale);
					model = glm::rotate(model, glm::radians(renderable.Rotation.x), { 1,0,0 });
					model = glm::rotate(model, glm::radians(renderable.Rotation.y), { 0,1,0 });
					model = glm::rotate(model, glm::radians(renderable.Rotation.z), { 0,0,1 });
					const glm::mat4x4 mvp = glm::transpose(projection * view * model);
					const VertexTransformationBuffer vertexUniformBufferData = { mvp,model };

					pDevice->UpdateHostBuffer(renderable.pTransformStageBuffer, (const unsigned char*)&vertexUniformBufferData, sizeof(vertexUniformBufferData), 0);
				}
			}

			//Begin cmd
			pCmdList->Begin();

			//Update transform buffers
			for (const MaterialData& material : materials)
			{
				for (const RenderableObject& renderable : material.Renderables)
				{
					pCmdList->CopyBufferBuffer(renderable.pTransformStageBuffer, renderable.pTransformBuffer, 0, 0, renderable.pTransformBuffer->GetSize());
				}
			}

			//Update camera view buffer
			pCmdList->CopyBufferBuffer(pCameraViewStageBuffer, pCameraViewBuffer, 0, 0, sizeof(pCameraViewBuffer->GetSize()));

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

			VkClearValue depthClearValue = {};
			depthClearValue.depthStencil.depth = 1.0f;
			pCmdList->SetDynamicRenderingDepthAttachment(pDepthTextureView, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, VkResolveModeFlags(), nullptr, VK_IMAGE_LAYOUT_UNDEFINED, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE, depthClearValue);

			//Start dynamic rendering
			pCmdList->BeginDynamicRendering(0, 1, 0, 0, pWindow->GetWidth(), pWindow->GetHeight());

			//Set pipeline
			pCmdList->SetPipeline(pPipeline);

			//Set static descriptor set
			pCmdList->SetDescriptorSets((const DescriptorSet**)&pStaticDescriptorSet, 1, 2, nullptr, 0);

			//Render renderables
			for (const MaterialData& material : materials)
			{
				//Set material descriptor set
				pCmdList->SetDescriptorSets((const DescriptorSet**)&material.pDescriptorSet, 1, 1, nullptr, 0);

				for (const RenderableObject& renderable : material.Renderables)
				{
					//Set renderable descriptor set
					pCmdList->SetDescriptorSets((const DescriptorSet**)&renderable.pDescriptorSet,1,0,nullptr,0);

					//Set vertex index buffers
					const unsigned long long offset = 0;
					pCmdList->SetVertexBuffers((const GraphicsBuffer**)&renderable.pVertexBuffer, 1, 0,&offset);
					pCmdList->SetIndexBuffer(renderable.pIndexBuffer, 0, VK_INDEX_TYPE_UINT32);

					//Draw
					pCmdList->DrawIndexed(renderable.pIndexBuffer->GetSize() / sizeof(unsigned int), 1, 0, 0, 0);
				}
			}

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
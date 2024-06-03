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


#define INSTANCE_COUNT 500

namespace Oksijen
{
	static const char vertexShaderSource[] =
		R"STR(
			#version 430 core
			layout(location = 0) in vec3 Pos;
			layout(location = 1) in vec2 UvIn;
			layout(location = 0) out vec2 UvOut;
			layout(std140,binding = 0,set = 0) uniform VertexBufferData
			{
				mat4 Mvp[500];
			};
			
			void main()
			{
				gl_Position = vec4(Pos,1.0f)*Mvp[gl_InstanceIndex];
				UvOut = UvIn;
			}
)STR";

	static const char pixelShaderSource[] =
		R"STR(
            #version 430 core
			layout(location = 0) in vec2 Uv;
			layout(location = 0) out vec4 out_col;
			
			layout(binding = 1,set = 0) uniform texture2D texture0;
			layout(binding = 2,set = 0) uniform sampler sampler0;
			void main()
			{
				vec4 color = texture(sampler2D(texture0,sampler0),Uv) + vec4(Uv.x,Uv.y,0,1.0f);
				out_col = color;
			}
)STR";

	struct InstanceData
	{
		glm::mat4x4 Mvp;
	};
	struct TransformData
	{
		glm::vec3 Position;
		glm::vec3 Scale;
		glm::vec3 Rotation;
		glm::vec3 PositionVelocity;
		glm::vec3 RotationVelocity;
	};

	void Run()
	{
		//Setup scene
		//Setup scene
		std::vector<TransformData> transformData(INSTANCE_COUNT);
		std::vector<InstanceData> sceneInstanceData(INSTANCE_COUNT);
		for (unsigned int i = 0; i < INSTANCE_COUNT; i++)
		{
			TransformData transform = {};
			transform.Position =
			{
				(rand() % 5) / 10.0f,
				(rand() % 5) / 10.0f,
				(rand() % 5) / 10.0f
			};
			transform.Rotation =
			{
				0,
				0,
				0
			};
			const float uniformScale = (rand() % 10) / 10.0f;
			transform.Scale =
			{
				uniformScale,
				uniformScale,
				uniformScale
			};
			transform.PositionVelocity =
			{
				(rand() % 5) / 100.0f,
				(rand() % 5) / 100.0f,
				(rand() % 5) / 100.0f
			};
			transform.RotationVelocity =
			{
				(rand() % 360) / 90.0f,
				(rand() % 360) / 90.0f,
				(rand() % 360) / 90.0f
			};

			transformData[i] = transform;
		}

		//Get monitor
		PlatformMonitor* pMonitor = PlatformMonitor::GetMonitors()[1];

		//Create window
		WindowDesc windowDesc = {};
		windowDesc.X = 0;
		windowDesc.Y = 0;
		windowDesc.Width = 512;
		windowDesc.Height = 512;
		windowDesc.Title = "04_NaiveInstancing";

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
		DEV_ASSERT(ShaderCompiler::TextToSPIRV(pixelShaderSource, "main", shaderc_fragment_shader, shaderc_source_language_glsl, &pFragmentShaderSPIRVBytes, fragmentShaderSPIRVByteCount, fragmentShaderCompilationErrors), "Main", "Failed to compile the fragment shader");

		//Create shaders
		Shader* pVertexShader = pDevice->CreateShader("main",VK_SHADER_STAGE_VERTEX_BIT,pVertexShaderSPIRVBytes, vertexShaderSPIRVByteCount);
		Shader* pFragmentShader = pDevice->CreateShader("main",VK_SHADER_STAGE_FRAGMENT_BIT, pFragmentShaderSPIRVBytes, fragmentShaderSPIRVByteCount);

		//Create command pool
		CommandPool* pCmdPool = pDevice->CreateCommandPool(VK_QUEUE_GRAPHICS_BIT);

		//Create command list
		CommandList* pCmdList = pDevice->AllocateCommandList(pCmdPool);

		//Create general use-case fence
		Fence* pCmdFence = pDevice->CreateFence(false);

		//Allocate memory
		GraphicsMemory* pHostMemory = pDevice->AllocateMemory(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, MB_TO_BYTE(50));
		GraphicsMemory* pDeviceMemory = pDevice->AllocateMemory(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, MB_TO_BYTE(50));

		//Create host buffer
		GraphicsBuffer* pHostBuffer = pDevice->CreateBuffer(pHostMemory, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_EXCLUSIVE, MB_TO_BYTE(49));

		//Create vertex buffer
		struct Vertex
		{
			float X;
			float Y;
			float Z;

			float U;
			float V;
		};
		constexpr Vertex vertexes[] =
		{
			{-1.0f,-1.0f,1.0f,0,0},
			{-1.0f,1.0f,1.0f,0.0f,1.0f},
			{1.0f,1.0f,1.0f,1.0f,1.0f},
			{1.0f,-1.0f,1.0f,1.0f,0.0f},

			{-1.0f,-1.0f,-1.0f,0,0},
			{-1.0f,1.0f,-1.0f,0.0f,1.0f},
			{1.0f,1.0f,-1.0f,1.0f,1.0f},
			{1.0f,-1.0f,-1.0f,1.0f,0.0f},

			{1.0f,1.0f,-1.0f,0,0},
			{1.0f,-1.0f,-1.0f,0.0f,1.0f},
			{1.0f,-1.0f,1.0f,1.0f,1.0f},
			{1.0f,1.0f,1.0f,1.0f,0.0f},

			{-1.0f,1.0f,-1.0f,0,0},
			{-1.0f,-1.0f,-1.0f,0.0f,1.0f},
			{-1.0f,-1.0f,1.0f,1.0f,1.0f},
			{-1.0f,1.0f,1.0f,1.0f,0.0f},

			{-1.0f,1.0f,1.0f,0,0},
			{-1.0f,1.0f,-1.0f,0.0f,1.0f},
			{1.0f,1.0f,-1.0f,1.0f,1.0f},
			{1.0f,1.0f,1.0f,1.0f,0.0f},

			{-1.0f,-1.0f,1.0f,0,0},
			{-1.0f,-1.0f,-1.0f,0.0f,1.0f},
			{1.0f,-1.0f,-1.0f,1.0f,1.0f},
			{1.0f,-1.0f,1.0f,1.0f,0.0f}
		};

		GraphicsBuffer* pVertexBuffer = pDevice->CreateBuffer(pDeviceMemory, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_SHARING_MODE_EXCLUSIVE, sizeof(vertexes));

		//Update vertex buffer
		pDevice->UpdateHostBuffer(pHostBuffer, (const unsigned char*)vertexes, sizeof(vertexes), 0);
		pCmdList->Begin();
		pCmdList->CopyBufferBuffer(pHostBuffer, pVertexBuffer, 0, 0,sizeof(vertexes));
		pCmdList->End();
		pDevice->SubmitCommandLists(pDefaultGraphicsQueue, (const CommandList**)&pCmdList, 1, nullptr, pCmdFence, nullptr, 0, nullptr, 0);
		pCmdFence->Wait();
		pCmdFence->Reset();

		//Create index buffer
		constexpr unsigned short indexes[] =
		{
			0,1,2,
			0,2,3,

			4,5,6,
			4,6,7,

			8,9,10,
			8,10,11,

			12,13,14,
			12,14,15,

			16,17,18,
			16,18,19,

			20,21,22,
			20,22,23
		};

		GraphicsBuffer* pIndexBuffer = pDevice->CreateBuffer(pDeviceMemory, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_SHARING_MODE_EXCLUSIVE, sizeof(indexes));

		//Update index buffer
		pDevice->UpdateHostBuffer(pHostBuffer, (const unsigned char*)indexes, sizeof(indexes), 0);
		pCmdList->Begin();
		pCmdList->CopyBufferBuffer(pHostBuffer, pIndexBuffer, 0, 0, sizeof(indexes));
		pCmdList->End();
		pDevice->SubmitCommandLists(pDefaultGraphicsQueue, (const CommandList**)&pCmdList, 1, nullptr, pCmdFence, nullptr, 0, nullptr, 0);
		pCmdFence->Wait();
		pCmdFence->Reset();

		//Create uniform buffer
		struct UniformBufferData
		{
			glm::mat4x4 Mvp;
		};
		GraphicsBuffer* pUniformBuffer = pDevice->CreateBuffer(pDeviceMemory, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE, sizeof(UniformBufferData)*INSTANCE_COUNT);

		//Load texture
		unsigned long long textureDataSize = 0;
		unsigned int textureWidth = 0;
		unsigned int textureHeight = 0;
		unsigned int textureChannelCount = 0;
		std::string texturePath = RES_PATH;
		texturePath += "/Smiley.png";
		unsigned char* pTextureData = nullptr;
		DEV_ASSERT(TextureLoader::LoadFromPath(texturePath,4,&pTextureData,textureDataSize,textureWidth,textureHeight, textureChannelCount), "Main", "Failed to load the test texture");

		//Create texture
		Texture* pTexture = pDevice->CreateTexture(
			pDeviceMemory,
			VK_IMAGE_TYPE_2D,
			VK_FORMAT_R8G8B8A8_UNORM,
			textureWidth,textureHeight,1,
			1,1,
			VK_SAMPLE_COUNT_1_BIT,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
			VK_SHARING_MODE_EXCLUSIVE,
			VK_IMAGE_LAYOUT_UNDEFINED);

		//Update texture
		pDevice->UpdateHostBuffer(pHostBuffer, pTextureData, textureDataSize, 0);
		pCmdList->Begin();
		pCmdList->SetPipelineTextureBarrier(pTexture,
			VK_IMAGE_LAYOUT_UNDEFINED,VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_QUEUE_GRAPHICS_BIT,VK_QUEUE_GRAPHICS_BIT,
			VK_IMAGE_ASPECT_COLOR_BIT,0,0,
			VK_ACCESS_NONE,VK_ACCESS_TRANSFER_WRITE_BIT,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,VK_PIPELINE_STAGE_TRANSFER_BIT,
			VkDependencyFlags());

		pCmdList->CopyBufferTexture(
			pHostBuffer, pTexture,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			0, 0, 0,
			VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1,
			0, 0, 0,
			textureWidth, textureHeight, 1
		);

		pCmdList->SetPipelineTextureBarrier(pTexture,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_QUEUE_GRAPHICS_BIT, VK_QUEUE_GRAPHICS_BIT,
			VK_IMAGE_ASPECT_COLOR_BIT, 0, 0,
			VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			VkDependencyFlags());

		pCmdList->End();
		pDevice->SubmitCommandLists(pDefaultGraphicsQueue, (const CommandList**)&pCmdList, 1, nullptr, pCmdFence, nullptr, 0, nullptr, 0);
		pCmdFence->Wait();
		pCmdFence->Reset();

		//Create texture view
		TextureView* pTextureView = pDevice->CreateTextureView(
			pTexture,
			0, 0,
			VK_IMAGE_VIEW_TYPE_2D,
			VK_FORMAT_R8G8B8A8_UNORM,
			{ VK_COMPONENT_SWIZZLE_IDENTITY,VK_COMPONENT_SWIZZLE_IDENTITY,VK_COMPONENT_SWIZZLE_IDENTITY,VK_COMPONENT_SWIZZLE_IDENTITY},
			VK_IMAGE_ASPECT_COLOR_BIT);

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

		//Set texture layout
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

		//Create depth texture view
		TextureView* pDepthTextureView = pDevice->CreateTextureView(
			pDepthTexture,
			0, 0,
			VK_IMAGE_VIEW_TYPE_2D,
			VK_FORMAT_D32_SFLOAT, { VK_COMPONENT_SWIZZLE_IDENTITY,VK_COMPONENT_SWIZZLE_IDENTITY,VK_COMPONENT_SWIZZLE_IDENTITY,VK_COMPONENT_SWIZZLE_IDENTITY },
			VK_IMAGE_ASPECT_DEPTH_BIT);

		//Create sampler
		Sampler* pSampler = pDevice->CreateSampler(
			VK_FILTER_LINEAR,VK_FILTER_NEAREST,
			VK_SAMPLER_MIPMAP_MODE_LINEAR,
			VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT,
			0,
			false,
			0,
			false,
			VK_COMPARE_OP_NEVER,
			0,0,VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK,false
		);

		//Create descriptor pool
		constexpr VkDescriptorType descriptorTypes[] = {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,VK_DESCRIPTOR_TYPE_SAMPLER};
		constexpr unsigned int descriptorTypeCounts[] = { 1,1,1 };
		DescriptorPool* pDescriptorPool = pDevice->CreateDescriptorPool(
			VkDescriptorPoolCreateFlags(),
			1,
			descriptorTypes,
			descriptorTypeCounts,
			2);

		//Create descriptor layout
		constexpr unsigned int descriptorBindingIndexes[3] = { 0,1,2 };
		constexpr VkDescriptorType descriptorLayoutTypes[3] = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,VK_DESCRIPTOR_TYPE_SAMPLER };
		constexpr unsigned int descriptorLayoutDescriptorCounts[3] = { 1,1,1 };
		constexpr VkShaderStageFlags descriptorLayoutShaderStageFlags[3] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT,VK_SHADER_STAGE_FRAGMENT_BIT };
		DescriptorSetLayout* pDescriptorLayout = pDevice->CreateDescriptorSetLayout(
			descriptorBindingIndexes,
			descriptorLayoutTypes,
			descriptorLayoutDescriptorCounts,
			descriptorLayoutShaderStageFlags,
			3
		);

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

		VkVertexInputAttributeDescription uvAttributeDesc = {};
		uvAttributeDesc.binding = 0;
		uvAttributeDesc.format = VK_FORMAT_R32G32_SFLOAT;
		uvAttributeDesc.location = 1;
		uvAttributeDesc.offset = sizeof(float) * 3;

		VkVertexInputAttributeDescription attributes[] = { posAttributeDesc,uvAttributeDesc };
		vertexInputState.vertexAttributeDescriptionCount = 2;
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
		rasterizerDesc.cullMode = VK_CULL_MODE_NONE;
		rasterizerDesc.depthBiasClamp = 0;
		rasterizerDesc.depthBiasConstantFactor = 0;
		rasterizerDesc.depthBiasEnable = VK_FALSE;
		rasterizerDesc.depthBiasSlopeFactor = 0;
		rasterizerDesc.depthClampEnable = VK_FALSE;
		rasterizerDesc.flags = 0;
		rasterizerDesc.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
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
		colorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT| VK_COLOR_COMPONENT_A_BIT;

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
			(const DescriptorSetLayout**)&pDescriptorLayout, 1,
			nullptr,
			nullptr, 0,
			&renderingInfo);

		//Allocate descriptor set
		DescriptorSet* pDescriptorSet = pDevice->AllocateDescriptorSet(pDescriptorPool, pDescriptorLayout);

		//Update descriptor set
		pDevice->UpdateDescriptorSetBuffer(
			pDescriptorSet,
			0,
			0,
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			pUniformBuffer,
			0,
			pUniformBuffer->GetSize()
		);

		pDevice->UpdateDescriptorSetTextureSampler(
			pDescriptorSet,
			1, 0,
			VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
			pTextureView, nullptr,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		pDevice->UpdateDescriptorSetTextureSampler(pDescriptorSet,
			2, 0,
			VK_DESCRIPTOR_TYPE_SAMPLER,
			nullptr,
			pSampler,
			VK_IMAGE_LAYOUT_UNDEFINED);


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

		//Application loop
		bool bRed = true;
		const glm::vec3 cubePosition = { 0,0,0 };
		const glm::vec3 cubeScale = { 1,1,1 };
		glm::vec3 cubeRotation = { 0,0,0 };
		const glm::vec3 cameraPosition = { 50,0,-50 };
		const glm::vec3 relativeUp = { 0,1,0 };
		while (pWindow->IsActive())
		{
			// poll window events
			pWindow->PollEvents();
			if (!pWindow->IsActive())
			{
				break;
			}

			//Update instanceData
			const float aspectRatio = pWindow->GetWidth() / pWindow->GetHeight();
			const glm::mat4x4 projection = glm::perspective(glm::radians(60.0f), aspectRatio, 0.01f, 100.0f);
			const glm::mat4x4 view = glm::lookAt(cameraPosition, { 32,20,0 }, { 0,1,0 });
			for (unsigned int i = 0; i < INSTANCE_COUNT; i++)
			{
				//Get
				TransformData& transform = transformData[i];
				InstanceData& instance = sceneInstanceData[i];

				//Update transform
				transform.Position += transform.PositionVelocity;
				transform.Rotation += transform.RotationVelocity;

				glm::mat4x4 model(1);
				model = glm::translate(model, transform.Position);
				model = glm::scale(model, transform.Scale);
				model = glm::rotate(model, glm::radians(transform.Rotation.x), { 1,0,0 });
				model = glm::rotate(model, glm::radians(transform.Rotation.y), { 0,1,0 });
				model = glm::rotate(model, glm::radians(transform.Rotation.z), { 0,0,1 });
				const glm::mat4x4 mvp = glm::transpose(projection * view * model);
				instance.Mvp = mvp;
			}

			//Update host stage buffer

			pDevice->UpdateHostBuffer(pHostBuffer, (const unsigned char*)sceneInstanceData.data(), INSTANCE_COUNT*sizeof(InstanceData), 0);

			//Acquire image index
			const unsigned int swapchainImageIndex = pSwapchain->AcquireImageIndex(pPresentImageAcquireFence, nullptr);
			pPresentImageAcquireFence->Wait();
			pPresentImageAcquireFence->Reset();

			//Begin cmd
			pCmdList->Begin();

			//Copy host to device (transform uniform buffer)
			pCmdList->CopyBufferBuffer(pHostBuffer, pUniformBuffer, 0, 0, INSTANCE_COUNT * sizeof(InstanceData));

			//Set pipeline
			pCmdList->SetPipeline(pPipeline);

			//Set vertex buffer
			constexpr unsigned long long vertexBufferOffsets = 0;
			pCmdList->SetVertexBuffers((const GraphicsBuffer**)&pVertexBuffer, 1, 0, &vertexBufferOffsets);

			//Set index buffer
			pCmdList->SetIndexBuffer(pIndexBuffer, 0, VK_INDEX_TYPE_UINT16);

			//Pre barrier
			pCmdList->SetPipelineTextureBarrier(
				pSwapchain->GetTexture(swapchainImageIndex),
				VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				VK_QUEUE_GRAPHICS_BIT, VK_QUEUE_GRAPHICS_BIT,
				VK_IMAGE_ASPECT_COLOR_BIT, 0, 0,
				VK_ACCESS_COLOR_ATTACHMENT_READ_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				VkDependencyFlags());

			//Set descriptor sets
			pCmdList->SetDescriptorSets((const DescriptorSet**)&pDescriptorSet, 1, 0, nullptr, 0);

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

			//Draw
			pCmdList->DrawIndexed(sizeof(indexes)/sizeof(unsigned short), INSTANCE_COUNT, 0, 0, 0);

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
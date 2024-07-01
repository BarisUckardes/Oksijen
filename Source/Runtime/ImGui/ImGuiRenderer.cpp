#include "ImGuiRenderer.h"
#include <Runtime/Graphics/Shader/ShaderCompiler.h>
#include <Runtime/ImGui/ImGuiUtils.h>

namespace Oksijen
{
	static const char vertexShaderSource[] =
		R"STR(
			#version 430 core
			layout(location = 0) in vec3 Pos;
			layout(location = 1) in vec2 UvIn;
			layout(location = 2) in vec4 Color;

			layout(location = 0) out vec2 UvOut;
			layout(location = 1) out vec4 ColorOut;

			layout(std140,binding = 0,set = 0) uniform VertexBufferData
			{
				mat4 ProjectionMatrix;
			};
			
			void main()
			{
				gl_Position = ProjectionMatrix*vec4(Pos,1.0f);
				UvOut = UvIn;
				ColorOut = Color;
			}
)STR";

	static const char fragmentShaderSource[] =
		R"STR(
			#version 430 core
			layout(location = 0) in vec2 Uv;
			layout(location = 1) in vec4 ColorIn;
			layout(location = 0) out vec4 ColorOut;

			layout(binding = 1,set = 0) uniform sampler sampler0;
			layout(binding = 0,set = 1) uniform texture2D texture0;

			void main()
			{
				vec4 textureColor = texture(sampler2D(texture0,sampler0),Uv);
				ColorOut = textureColor*ColorIn;
			}
		)STR";

	ImGuiRenderer::ImGuiRenderer(GraphicsDevice* pDevice,const GraphicsQueue* pQueue, const unsigned long long vertexBufferSize, const unsigned long long indexBufferSize, const unsigned long long fontTextureSize)
		: mDevice(pDevice),mVertexBufferSize(vertexBufferSize),mIndexBufferSize(indexBufferSize),mFontTextureSize(fontTextureSize), mMemoryCapacity(vertexBufferSize + indexBufferSize + fontTextureSize),
		mHostMemory(nullptr), mDeviceMemory(nullptr), mCmdPool(nullptr), mCmdList(nullptr), mFence(nullptr),mQueue(pQueue)
	{
		InitializeInternalResources();
	}
	ImGuiRenderer::~ImGuiRenderer()
	{

	}
	void ImGuiRenderer::UpdateEvents(const std::vector<WindowEventData>& events)
	{
		for (const WindowEventData& eventData : events)
		{
			switch (eventData.Type)
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
					OnKeyboardKeyDown(eventData.KeyboardKey);
					break;
				case Oksijen::WindowEventType::KeyboardUp:
					OnKeyboardKeyUp(eventData.KeyboardKey);
					break;
				case Oksijen::WindowEventType::Char:
					OnKeyboardChar(eventData.KeyboardChar);
					break;
				case Oksijen::WindowEventType::MouseButtonDown:
					OnMouseButtonDown(eventData.MouseButton);
					break;
				case Oksijen::WindowEventType::MouseButtonUp:
					OnMouseButtonUp(eventData.MouseButton);
					break;
				case Oksijen::WindowEventType::MouseMoved:
					OnMouseMoved(eventData.MousePosX, eventData.MousePosY);
					break;
				case Oksijen::WindowEventType::MouseScrolled:
					OnMouseWheel(eventData.MouseWheelDelta);
					break;
				default:
					break;
			}
		}
	}
	void ImGuiRenderer::StartRendering(const float deltaTimeInMilliseconds)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.DeltaTime = deltaTimeInMilliseconds;

		ImGui::NewFrame();
	}
	void ImGuiRenderer::EndRendering(
		const TextureView* pTargetView,
		const VkImageLayout oldImageLayout,
		const VkQueueFlags oldQueue,
		const VkImageAspectFlags imageAspectMask,
		const VkAccessFlags oldAccessMask,
		const VkPipelineStageFlags oldPipelineStageMask,
		const VkDependencyFlags dependencies)
	{
		DEV_ASSERT(pTargetView != nullptr, "ImGuiRenderer", "EndRendering pTargetView cannot be nullptr!");

		//Get target texture
		const Texture* pTargetTexture = pTargetView->GetTexture();

		//Set display size and scale
		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = { (float)pTargetTexture->GetWidth(),(float)pTargetTexture->GetHeight()};
		io.DisplayFramebufferScale = { 1.0f,1.0f };

		//Render queued calls
		ImGui::Render();

		//Get draw data
		ImDrawData* pDrawData = ImGui::GetDrawData();

		//Validate vertex and index buffers
		const unsigned long long sessionTotalVertexBufferSize = pDrawData->TotalVtxCount * sizeof(ImDrawVert);
		const unsigned long long sessionTotalIndexBufferSize = pDrawData->TotalIdxCount * sizeof(unsigned short);
		const unsigned long long sessionTotalBufferSize = sessionTotalVertexBufferSize + sessionTotalIndexBufferSize;
		DEV_ASSERT(sessionTotalVertexBufferSize <= mVertexBuffer->GetSize(), "ImGuiRenderer", "Submitted vertex buffer size %ld exceeds the internal buffer size %ld", sessionTotalVertexBufferSize, mVertexBufferSize);
		DEV_ASSERT(sessionTotalIndexBufferSize <= mIndexBuffer->GetSize(), "ImGuiRenderer", "Submitted index buffer size &ld sceeds the internal buffer size %ld", sessionTotalIndexBufferSize, mIndexBufferSize);

		//Update vertex host buffer data
		unsigned int vertexBufferOffset = 0;
		for (unsigned int i = 0; i < pDrawData->CmdListsCount; i++)
		{
			const ImDrawList* pCmdList = pDrawData->CmdLists[i];
			const unsigned int cmdListVertexBufferSize = pCmdList->VtxBuffer.Size * sizeof(ImDrawVert);

			mDevice->UpdateHostBuffer(mStagingBuffer, (const unsigned char*)pCmdList->VtxBuffer.Data, cmdListVertexBufferSize,vertexBufferOffset);

			vertexBufferOffset += cmdListVertexBufferSize;
		}

		//Update index host buffer data
		unsigned int indexBufferOffset = vertexBufferOffset;
		for (unsigned int i = 0; i < pDrawData->CmdListsCount; i++)
		{
			const ImDrawList* pCmdList = pDrawData->CmdLists[i];
			const unsigned int cmdListIndexBufferSize = pCmdList->IdxBuffer.Size * sizeof(unsigned short);

			mDevice->UpdateHostBuffer(mStagingBuffer, (const unsigned char*)pCmdList->IdxBuffer.Data, cmdListIndexBufferSize, indexBufferOffset);

			indexBufferOffset += cmdListIndexBufferSize;
		}

		//Create projection matrix
		const float L = pDrawData->DisplayPos.x;
		const float R = L + pDrawData->DisplaySize.x;
		const float B = pDrawData->DisplayPos.y;
		const float T = B + pDrawData->DisplaySize.y;
		const float projectionData[] =
		{
			2.0f / (R - L),0,0,0,
			0,2.0f / (T - B),0,0,
			0,0,0.5f,0,
			(R + L) / (L - R),(T + B) / (B - T),0.5f,1.0f
		};

		//Update projection matrix host data
		mDevice->UpdateHostBuffer(mStagingBuffer, (const unsigned char*)projectionData, sizeof(projectionData), indexBufferOffset);

		//Begin cmd recording
		mCmdList->Begin();

		//Update vertex buffer
		if(vertexBufferOffset > 0)
			mCmdList->CopyBufferBuffer(mStagingBuffer, mVertexBuffer, 0, 0, vertexBufferOffset);

		//Update index buffer
		if(indexBufferOffset-vertexBufferOffset > 0)
			mCmdList->CopyBufferBuffer(mStagingBuffer, mIndexBuffer, vertexBufferOffset, 0, indexBufferOffset - vertexBufferOffset);

		//Update projection data
		mCmdList->CopyBufferBuffer(mStagingBuffer, mTransformBuffer, indexBufferOffset, 0, 64);

		//Set pipeline
		mCmdList->SetPipeline(mPipeline);

		//Set vertex buffers
		constexpr unsigned long long vertexBufferOffsets = 0;
		mCmdList->SetVertexBuffers((const GraphicsBuffer**)&mVertexBuffer, 1, 0, &vertexBufferOffsets);

		//Set index buffer
		mCmdList->SetIndexBuffer(mIndexBuffer, 0, VK_INDEX_TYPE_UINT16);

		//Set clip rects
		pDrawData->ScaleClipRects(io.DisplayFramebufferScale);

		//Set viewports
		VkViewport viewport = {};
		viewport.x = 0;
		viewport.y = 0;
		viewport.width = pTargetTexture->GetWidth();
		viewport.height = pTargetTexture->GetHeight();
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		mCmdList->PlaceViewport(&viewport,1,0);

		//Set barrier
		mCmdList->SetPipelineTextureBarrier(
			pTargetTexture,
			oldImageLayout, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			oldQueue, VK_QUEUE_GRAPHICS_BIT,
			VK_IMAGE_ASPECT_COLOR_BIT, pTargetView->GetMipIndex(), pTargetView->GetArrayIndex(),
			oldAccessMask, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			oldPipelineStageMask, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VkDependencyFlags()
		);

		//Start rendering
		VkClearColorValue clearColorValue = {};
		clearColorValue.float32[0] = 0.0f;
		clearColorValue.float32[1] = 1.0f;
		clearColorValue.float32[2] = 0.0f;
		clearColorValue.float32[3] = 1.0f;

		mCmdList->AddDynamicRenderingColorAttachment(pTargetView, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VkResolveModeFlags(), nullptr, VK_IMAGE_LAYOUT_UNDEFINED, VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_STORE_OP_STORE, clearColorValue);

		mCmdList->BeginDynamicRendering(0, 1, 0, 0, pTargetTexture->GetWidth(), pTargetTexture->GetHeight());

		//Draw cmd lists
		unsigned int drawVertexOffset = 0;
		unsigned int drawIndexOffset = 0;
		DescriptorSet* ppDescriptorSets[] =
		{
			mStaticDescriptorSet,
			nullptr
		};

		const float clipOffset[2] =
		{
			pDrawData->DisplayPos.x,
			pDrawData->DisplayPos.y
		};
		for (unsigned int cmdListIndex = 0; cmdListIndex < pDrawData->CmdListsCount; cmdListIndex++)
		{
			const ImDrawList* pDrawList = pDrawData->CmdLists[cmdListIndex];
			for (unsigned int cmdIndex = 0; cmdIndex < pDrawList->CmdBuffer.Size; cmdIndex++)
			{
				const ImDrawCmd& cmd = pDrawList->CmdBuffer[cmdIndex];

				const float clipMin[2] =
				{
					cmd.ClipRect.x - clipOffset[0],
					cmd.ClipRect.y - clipOffset[1]
				};
				const float clipMax[2] =
				{
					cmd.ClipRect.z - clipOffset[0],
					cmd.ClipRect.w - clipOffset[1]
				};

				if (clipMax[0] <= clipMin[0] || clipMax[1] <= clipMin[1])
					continue;

				VkRect2D scissorRect = {};
				scissorRect.offset = { (int)clipMin[0],(int)clipMin[1] };
				scissorRect.extent = { (unsigned int)clipMax[0],(unsigned int)clipMax[1] };
				mCmdList->PlaceScissor(&scissorRect, 1, 0);

				if (cmd.TextureId == nullptr) // default font text
				{
					ppDescriptorSets[1] = mDefaultFontTextureDescriptorSet;
				}
				else
				{
					ppDescriptorSets[1] = (DescriptorSet*)cmd.TextureId;
				}

				mCmdList->SetDescriptorSets((const DescriptorSet**)ppDescriptorSets, 2, 0, nullptr, 0);
				mCmdList->DrawIndexed(cmd.ElemCount, 1, (drawIndexOffset + cmd.IdxOffset), (drawVertexOffset+cmd.VtxOffset), 0);
			}

			drawVertexOffset += pDrawList->VtxBuffer.Size;
			drawIndexOffset += pDrawList->IdxBuffer.Size;
		}

		mCmdList->EndDynamicRendering();

		//End cmd recording
		mCmdList->End();
		
		mDevice->SubmitCommandLists(mQueue, (const CommandList**)&mCmdList, 1, nullptr, mFence, nullptr, 0, nullptr, 0);
		mFence->Wait();
		mFence->Reset();

	}

	void ImGuiRenderer::InitializeInternalResources()
	{
		//Create imgui context
		mImGuiContext = ImGui::CreateContext();
		ImGui::SetCurrentContext(mImGuiContext);

		//Set IO flags
		ImGuiIO& io = ImGui::GetIO();
		io.BackendFlags = ImGuiBackendFlags_None;
		io.ConfigFlags = ImGuiConfigFlags_DockingEnable;
		io.DisplaySize = { 512.0f,512.0f };
		io.DisplayFramebufferScale = { 1.0f,1.0f };

		//Create cmd pool
		mCmdPool = mDevice->CreateCommandPool(VK_QUEUE_GRAPHICS_BIT);

		//Allocate command list
		mCmdList = mDevice->AllocateCommandList(mCmdPool);

		//Create fence
		mFence = mDevice->CreateFence(false);

		//Allocate memory
		mHostMemory = mDevice->AllocateMemory(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, mMemoryCapacity);
		mDeviceMemory = mDevice->AllocateMemory(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mMemoryCapacity);

		//Create staging buffer
		mStagingBuffer = mDevice->CreateBuffer(mHostMemory, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_EXCLUSIVE, mMemoryCapacity - MB_TO_BYTE(0.1f));

		//Create vertex buffer
		mVertexBuffer = mDevice->CreateBuffer(mDeviceMemory,VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,VK_SHARING_MODE_EXCLUSIVE,mVertexBufferSize);

		//Create index buffer
		mIndexBuffer = mDevice->CreateBuffer(mDeviceMemory, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE, mIndexBufferSize);

		//Create transferm buffer
		mTransformBuffer = mDevice->CreateBuffer(mDeviceMemory, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE, 64);

		//Create default font texture
		unsigned char* pFontData = nullptr;
		int width;
		int height;
		int channelCount;
		io.Fonts->GetTexDataAsRGBA32(&pFontData, &width, &height, &channelCount);

		DEV_ASSERT(pFontData != nullptr && width != 0 && height != 0 && channelCount != 0, "ImGuiRenderer", "Failed to initialize, invalid default font texture!");

		mDefaultFontTexture = mDevice->CreateTexture(mDeviceMemory,
			VK_IMAGE_TYPE_2D,
			VK_FORMAT_R8G8B8A8_UNORM,
			width, height, 1,
			1, 1,
			VK_SAMPLE_COUNT_1_BIT,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_SHARING_MODE_EXCLUSIVE,
			VK_IMAGE_LAYOUT_UNDEFINED);

		//Update default texture
		mDevice->UpdateHostBuffer(mStagingBuffer,pFontData,width*height*4,0);

		mCmdList->Begin();

		mCmdList->SetPipelineTextureBarrier(
			mDefaultFontTexture,
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_QUEUE_GRAPHICS_BIT, VK_QUEUE_GRAPHICS_BIT,
			VK_IMAGE_ASPECT_COLOR_BIT, 0, 0,
			VK_ACCESS_NONE, VK_ACCESS_TRANSFER_WRITE_BIT,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
			VkDependencyFlags()
		);

		mCmdList->CopyBufferTexture(
			mStagingBuffer, mDefaultFontTexture,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			0, 0, 0,
			VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1,
			0, 0, 0,
			width, height, 1);

		mCmdList->SetPipelineTextureBarrier(
			mDefaultFontTexture,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_QUEUE_GRAPHICS_BIT,VK_QUEUE_GRAPHICS_BIT,
			VK_IMAGE_ASPECT_COLOR_BIT,0,0,
			VK_ACCESS_TRANSFER_WRITE_BIT,VK_ACCESS_SHADER_READ_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT,VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			VkDependencyFlags()
		);

		mCmdList->End();
		mDevice->SubmitCommandLists(mQueue,(const CommandList**)&mCmdList,1,nullptr,mFence,nullptr,0,nullptr,0);
		mFence->Wait();
		mFence->Reset();

		//Create default font texture view
		constexpr VkComponentMapping textureComponentMapping = { VK_COMPONENT_SWIZZLE_IDENTITY ,VK_COMPONENT_SWIZZLE_IDENTITY ,VK_COMPONENT_SWIZZLE_IDENTITY ,VK_COMPONENT_SWIZZLE_IDENTITY };
		mDefaultFontTextureView = mDevice->CreateTextureView(
			mDefaultFontTexture,
			0,0,
			VK_IMAGE_VIEW_TYPE_2D,
			VK_FORMAT_R8G8B8A8_UNORM,
			textureComponentMapping,
			VK_IMAGE_ASPECT_COLOR_BIT
		);

		//Create sampler
		mSampler = mDevice->CreateSampler(
			VK_FILTER_LINEAR,VK_FILTER_NEAREST,
			VK_SAMPLER_MIPMAP_MODE_NEAREST,
			VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT,
			0,
			false,0,
			false,VK_COMPARE_OP_NEVER,
			0,0,
			VK_BORDER_COLOR_INT_OPAQUE_BLACK,
			false
		);


		//Compile shaders
		unsigned char* pVertexShaderSPIRVBytes = nullptr;
		unsigned int vertexShaderSPIRVByteCount = 0;
		std::string vertexShaderCompilationErrors;
		DEV_ASSERT(ShaderCompiler::TextToSPIRV(vertexShaderSource, "main", shaderc_vertex_shader, shaderc_source_language_glsl, &pVertexShaderSPIRVBytes, vertexShaderSPIRVByteCount, vertexShaderCompilationErrors), "Main", "Failed to compile vertex shader with logs: %s", vertexShaderCompilationErrors.c_str());

		unsigned char* pFragmentShaderSPIRVBytes = nullptr;
		unsigned int fragmentShaderSPIRVByteCount = 0;
		std::string fragmentShaderCompilationErrors;
		DEV_ASSERT(ShaderCompiler::TextToSPIRV(fragmentShaderSource, "main", shaderc_fragment_shader, shaderc_source_language_glsl, &pFragmentShaderSPIRVBytes, fragmentShaderSPIRVByteCount, fragmentShaderCompilationErrors), "Main", "Failed to compile the fragment shader with logs: %s",fragmentShaderCompilationErrors.c_str());

		//Create shaders
		mVertexShader = mDevice->CreateShader("main", VK_SHADER_STAGE_VERTEX_BIT, pVertexShaderSPIRVBytes, vertexShaderSPIRVByteCount);
		mFragmentShader = mDevice->CreateShader("main", VK_SHADER_STAGE_FRAGMENT_BIT, pFragmentShaderSPIRVBytes, fragmentShaderSPIRVByteCount);

		//Create descriptor pool
		constexpr VkDescriptorType poolDescriptorTypes[3] = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,VK_DESCRIPTOR_TYPE_SAMPLER,VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE };
		constexpr unsigned int poolDescriptorTypeCounts[3] = { 1,1,1 };
		mDescriptorPool = mDevice->CreateDescriptorPool(VkDescriptorPoolCreateFlags(),2,poolDescriptorTypes,poolDescriptorTypeCounts,3);

		//Create static descriptor layout
		constexpr unsigned int staticSetLayoutBindings[2] = { 0,1 };
		constexpr VkDescriptorType staticSetLayoutDescriptorTypes[2] = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,VK_DESCRIPTOR_TYPE_SAMPLER };
		constexpr unsigned int staticSetLayoutDescriptorTypeCounts[2] = { 1,1 };
		constexpr VkShaderStageFlags staticSetLayoutStages[2] = { VK_SHADER_STAGE_VERTEX_BIT,VK_SHADER_STAGE_FRAGMENT_BIT };
		mStaticSetLayout = mDevice->CreateDescriptorSetLayout(staticSetLayoutBindings,staticSetLayoutDescriptorTypes, staticSetLayoutDescriptorTypeCounts, staticSetLayoutStages,2);

		//Create dynamic descriptor layout
		constexpr unsigned int dynamicSetLayoutBindings[1] = { 0 };
		constexpr VkDescriptorType dynamicSetLayoutDescriptorTypes[1] = { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE };
		constexpr unsigned int dynamicSetLayoutDescriptorTypeCounts[1] = { 1 };
		constexpr VkShaderStageFlags dynamicSetLayoutStages[1] = { VK_SHADER_STAGE_FRAGMENT_BIT };
		mDynamicSetLayout = mDevice->CreateDescriptorSetLayout(dynamicSetLayoutBindings, dynamicSetLayoutDescriptorTypes, dynamicSetLayoutDescriptorTypeCounts, dynamicSetLayoutStages, 1);

		//Create pipeline
		VkPipelineVertexInputStateCreateInfo vertexInputState = {};
		vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputState.flags = 0;

		VkVertexInputBindingDescription bindingDesc = {};
		bindingDesc.binding = 0;
		bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		bindingDesc.stride = sizeof(ImDrawVert);

		vertexInputState.vertexBindingDescriptionCount = 1;
		vertexInputState.pVertexBindingDescriptions = &bindingDesc;

		VkVertexInputAttributeDescription posAttributeDesc = {};
		posAttributeDesc.binding = 0;
		posAttributeDesc.format = VK_FORMAT_R32G32_SFLOAT;
		posAttributeDesc.location = 0;
		posAttributeDesc.offset = 0;

		VkVertexInputAttributeDescription uvAttributeDesc = {};
		uvAttributeDesc.binding = 0;
		uvAttributeDesc.format = VK_FORMAT_R32G32_SFLOAT;
		uvAttributeDesc.location = 1;
		uvAttributeDesc.offset = sizeof(float) * 2;

		VkVertexInputAttributeDescription colorAttributeDesc = {};
		colorAttributeDesc.binding = 0;
		colorAttributeDesc.format = VK_FORMAT_R8G8B8A8_UNORM;
		colorAttributeDesc.location = 2;
		colorAttributeDesc.offset = sizeof(float) * 4;

		VkVertexInputAttributeDescription attributes[] = { posAttributeDesc,uvAttributeDesc,colorAttributeDesc };
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
		rasterizerDesc.cullMode = VK_CULL_MODE_NONE;
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
		depthStencilDesc.depthTestEnable = VK_FALSE;
		depthStencilDesc.depthWriteEnable = VK_FALSE;
		depthStencilDesc.depthCompareOp = VK_COMPARE_OP_LESS;
		depthStencilDesc.depthBoundsTestEnable = VK_FALSE;
		depthStencilDesc.maxDepthBounds = 1.0f;
		depthStencilDesc.minDepthBounds = 0.0f;
		depthStencilDesc.pNext = nullptr;

		VkPipelineColorBlendStateCreateInfo blendingDesc = {};
		blendingDesc.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;

		VkPipelineColorBlendAttachmentState colorBlendAttachmentState = {};
		colorBlendAttachmentState.blendEnable = VK_TRUE;
		colorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		colorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		colorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		colorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

		blendingDesc.attachmentCount = 1;
		blendingDesc.pAttachments = &colorBlendAttachmentState;
		blendingDesc.logicOp = VK_LOGIC_OP_CLEAR;
		blendingDesc.logicOpEnable = VK_FALSE;
		blendingDesc.pNext = nullptr;

		constexpr VkFormat colorAttachmentFormat = VK_FORMAT_R8G8B8A8_UNORM;
		VkPipelineRenderingCreateInfoKHR renderingInfo = {};
		renderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
		renderingInfo.colorAttachmentCount = 1;
		renderingInfo.pColorAttachmentFormats = &colorAttachmentFormat;
		renderingInfo.depthAttachmentFormat = VK_FORMAT_UNDEFINED;
		renderingInfo.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;
		renderingInfo.pNext = nullptr;

		VkPipelineViewportStateCreateInfo viewportDesc = {};
		viewportDesc.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportDesc.scissorCount = 1;
		viewportDesc.viewportCount = 1;
		viewportDesc.pViewports = nullptr;
		viewportDesc.pScissors = nullptr;
		viewportDesc.pNext = nullptr;

		constexpr VkDynamicState dynamicStates[] =
		{
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};
		VkPipelineDynamicStateCreateInfo dynamicStateInfo = {};
		dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicStateInfo.flags = VkPipelineDynamicStateCreateFlags();
		dynamicStateInfo.pDynamicStates = dynamicStates;
		dynamicStateInfo.dynamicStateCount = sizeof(dynamicStates) / sizeof(VkDynamicState);
		dynamicStateInfo.pNext = nullptr;

		const DescriptorSetLayout* ppSetLayouts[] = { mStaticSetLayout,mDynamicSetLayout };
		const Shader* pPipelineShaders[] = { mVertexShader,mFragmentShader };
		mPipeline = mDevice->CreateGraphicsPipeline(
			vertexInputState,
			inputAssemblyState,
			nullptr,
			&viewportDesc,
			rasterizerDesc,
			multisampleDesc,
			depthStencilDesc,
			blendingDesc,
			&dynamicStateInfo,
			pPipelineShaders, 2,
			(const DescriptorSetLayout**)ppSetLayouts, 2,
			nullptr,
			nullptr, 0,
			&renderingInfo);

		//Create descriptor sets
		mStaticDescriptorSet = mDevice->AllocateDescriptorSet(mDescriptorPool, mStaticSetLayout);
		mDefaultFontTextureDescriptorSet = mDevice->AllocateDescriptorSet(mDescriptorPool, mDynamicSetLayout);

		//Update descriptor sets
		mDevice->UpdateDescriptorSetBuffer(mStaticDescriptorSet, 0, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, mTransformBuffer, 0, 64);
		mDevice->UpdateDescriptorSetTextureSampler(mStaticDescriptorSet, 1, 0, VK_DESCRIPTOR_TYPE_SAMPLER, nullptr, mSampler, VK_IMAGE_LAYOUT_UNDEFINED);
		mDevice->UpdateDescriptorSetTextureSampler(mDefaultFontTextureDescriptorSet, 0, 0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, mDefaultFontTextureView, nullptr, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}
	void ImGuiRenderer::OnMouseMoved(const int x, const int y)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.AddMousePosEvent(x,y);
	}
	void ImGuiRenderer::OnMouseButtonDown(const MouseButtons button)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.AddMouseButtonEvent(ImGuiUtils::GetMouseButton(button), true);
	}
	void ImGuiRenderer::OnMouseButtonUp(const MouseButtons button)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.AddMouseButtonEvent(ImGuiUtils::GetMouseButton(button), false);
	}
	void ImGuiRenderer::OnMouseWheel(const float delta)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.AddMouseWheelEvent(delta, delta);
	}
	void ImGuiRenderer::OnKeyboardKeyDown(const KeyboardKeys key)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.AddKeyEvent(ImGuiUtils::GetKeyboardKey(key), true);

		if (key == KeyboardKeys::LeftControl)
			io.AddKeyEvent(ImGuiKey_ModCtrl, true);
		if (key == KeyboardKeys::LeftShift)
			io.AddKeyEvent(ImGuiKey_ModShift, true);
		if (key == KeyboardKeys::LeftShift)
			io.AddKeyEvent(ImGuiKey_ModAlt, true);
		if (key == KeyboardKeys::LeftShift)
			io.AddKeyEvent(ImGuiKey_ModSuper, true);
	}
	void ImGuiRenderer::OnKeyboardKeyUp(const KeyboardKeys key)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.AddKeyEvent(ImGuiUtils::GetKeyboardKey(key), false);

		if (key == KeyboardKeys::LeftControl)
			io.AddKeyEvent(ImGuiKey_ModCtrl, false);
		if (key == KeyboardKeys::LeftShift)
			io.AddKeyEvent(ImGuiKey_ModShift, false);
		if (key == KeyboardKeys::LeftShift)
			io.AddKeyEvent(ImGuiKey_ModAlt, false);
		if (key == KeyboardKeys::LeftShift)
			io.AddKeyEvent(ImGuiKey_ModSuper, false);
	}
	void ImGuiRenderer::OnKeyboardChar(const char value)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.AddInputCharacter(value);
	}
}
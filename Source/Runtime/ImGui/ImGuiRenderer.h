#pragma once
#include <Runtime/Graphics/Device/GraphicsDevice.h>
#include <Runtime/Platform/PlatformWindow.h>
#include <imgui.h>

namespace Oksijen
{
	class RUNTIME_API ImGuiRenderer final
	{
	public:
		ImGuiRenderer(GraphicsDevice* pDevice,const GraphicsQueue* pQueue,const unsigned long long vertexBufferSize = MB_TO_BYTE(50), const unsigned long long indexBufferSize = MB_TO_BYTE(50), const unsigned long long fontTextureSize = MB_TO_BYTE(50));
		~ImGuiRenderer();

		void UpdateEvents(const std::vector<WindowEventData>& events);

		void StartRendering(const float deltaTimeInMilliseconds);
		void EndRendering(
			const TextureView* pTargetView,
			const VkImageLayout oldImageLayout,
			const VkQueueFlags oldQueue,
			const VkImageAspectFlags imageAspectMask,
			const VkAccessFlags oldAccessMask,
			const VkPipelineStageFlags oldPipelineStageMask,
			const VkDependencyFlags dependencies);
	private:
		void InitializeInternalResources();
		void OnMouseMoved(const int x, const int y);
		void OnMouseButtonDown(const MouseButtons button);
		void OnMouseButtonUp(const MouseButtons button);
		void OnMouseWheel(const float delta);
		void OnKeyboardKeyDown(const KeyboardKeys key);
		void OnKeyboardKeyUp(const KeyboardKeys key);
		void OnKeyboardChar(const char value);

	private:
		const unsigned long long mMemoryCapacity;
		const unsigned long long mVertexBufferSize;
		const unsigned long long mIndexBufferSize;
		const unsigned long long mFontTextureSize;
		const GraphicsQueue* mQueue;
		ImGuiContext* mImGuiContext;

		Shader* mVertexShader;
		Shader* mFragmentShader;

		GraphicsDevice* mDevice;
		GraphicsMemory* mHostMemory;
		GraphicsMemory* mDeviceMemory;

		GraphicsBuffer* mStagingBuffer;
		GraphicsBuffer* mVertexBuffer;
		GraphicsBuffer* mIndexBuffer;
		GraphicsBuffer* mTransformBuffer;

		Texture* mDefaultFontTexture;
		TextureView* mDefaultFontTextureView;
		Sampler* mSampler;

		DescriptorPool* mDescriptorPool;
		DescriptorSetLayout* mStaticSetLayout;
		DescriptorSetLayout* mDynamicSetLayout;
		DescriptorSet* mStaticDescriptorSet;
		DescriptorSet* mDefaultFontTextureDescriptorSet;

		Pipeline* mPipeline;

		CommandPool* mCmdPool;
		CommandList* mCmdList;
		Fence* mFence;
	};
}
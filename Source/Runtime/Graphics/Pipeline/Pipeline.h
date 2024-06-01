#pragma once
#include <Runtime/Core/Core.h>
#include <vulkan/vulkan.h>


namespace Oksijen
{
	class GraphicsDevice;
	class RUNTIME_API Pipeline
	{
	public:
		Pipeline(const GraphicsDevice* pDevice,const VkPipelineBindPoint bindPoint);
		~Pipeline();

		FORCEINLINE VkPipeline GetPipeline() const noexcept { return mPipeline; }
		FORCEINLINE VkPipelineLayout GetLayout() const noexcept { return mLayout; }
		FORCEINLINE VkPipelineBindPoint GetBindPoint() const noexcept { return mBindPoint; }
	protected:
		void SetPipelineProperties(const VkPipeline pipeline, const VkPipelineLayout layout) { mPipeline = pipeline; mLayout = layout; }
	private:
		const VkDevice mLogicalDevice;
		const VkPipelineBindPoint mBindPoint;
		VkPipeline mPipeline;
		VkPipelineLayout mLayout;
	};
}
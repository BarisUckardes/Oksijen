#pragma once
#include <Runtime/Core/Core.h>
#include <vulkan/vulkan.h>

namespace Oksijen
{
	class GraphicsDevice;
	class RUNTIME_API RenderPass final
	{
	public:
		RenderPass(const GraphicsDevice* pDevice);
		~RenderPass();

		FORCEINLINE VkRenderPass GetRenderPass() const noexcept { return mPass; }
	private:
		const VkDevice mLogicalDevice;
		VkRenderPass mPass;

	};
}
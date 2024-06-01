#include "RenderPass.h"
#include <Runtime/Graphics/Device/GraphicsDevice.h>

namespace Oksijen
{
	RenderPass::RenderPass(const GraphicsDevice* pDevice) : mLogicalDevice(pDevice->GetLogicalDevice())
	{
	}
	RenderPass::~RenderPass()
	{
		vkDestroyRenderPass(mLogicalDevice,mPass,nullptr);
	}
}
#include "Pipeline.h"
#include <Runtime/Graphics/Device/GraphicsDevice.h>

namespace Oksijen
{
	Pipeline::Pipeline(const GraphicsDevice* pDevice,const VkPipelineBindPoint bindPoint) 
		: mLogicalDevice(pDevice->GetLogicalDevice()),mPipeline(VK_NULL_HANDLE), mBindPoint(bindPoint)
	{

	}
	Pipeline::~Pipeline()
	{
		vkDestroyPipeline(mLogicalDevice, mPipeline, nullptr);
	}
}
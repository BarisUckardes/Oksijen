#include "GraphicsQueue.h"
#include <Runtime/Graphics/Device/GraphicsDevice.h>

namespace Oksijen
{
	GraphicsQueue::~GraphicsQueue()
	{
		//Inform device
		mOwnerDevice->SignalQueueReturned(mQueue, mFamilyIndex);
	}
	GraphicsQueue::GraphicsQueue(const VkQueue queue, const VkQueueFlags flags, const unsigned int familyIndex,GraphicsDevice* pDevice) : mQueue(queue),mFlags(flags),mFamilyIndex(familyIndex),mOwnerDevice(pDevice)
	{

	}
}
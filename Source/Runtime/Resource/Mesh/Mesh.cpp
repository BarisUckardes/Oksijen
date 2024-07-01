#include "Mesh.h"
#include <Runtime/Graphics/Device/GraphicsDevice.h>

namespace Oksijen
{
	unsigned int CalculateBytesPerVertex(const MeshLayout& layout)
	{
		return 0;
	}
	Mesh::Mesh(GraphicsDevice* pDevice,GraphicsMemory* pHostMemory, GraphicsMemory* pDeviceMemory, const MeshLayout& layout, const unsigned char bytesPerIndex) 
		: mDevice(pDevice), mLayout(layout), mBytesPerIndex(bytesPerIndex), mBytesPerVertex(CalculateBytesPerVertex(layout)), mHostMemory(pHostMemory), mDeviceMemory(pDeviceMemory),
		mIndexHostBuffer(nullptr),mIndexDeviceBuffer(nullptr),mIndexCount(0),mVertexCount(0)
	{
		mCmdPool = pDevice->CreateCommandPool(VK_QUEUE_GRAPHICS_BIT);
		mCmdList = pDevice->AllocateCommandList(mCmdPool);
		mFence = pDevice->CreateFence(true);
	}
	void Mesh::AllocateVertexes(const unsigned long long count)
	{
	}
	void Mesh::AllocateIndexes(const unsigned long long count)
	{
		mIndexCount = count;

		//Free former indexes
		FreeIndexes();

		//Allocate buffers
		mIndexHostBuffer = mDevice->CreateBuffer(mHostMemory, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_EXCLUSIVE, count * mBytesPerIndex);
	}
	void Mesh::UpdateVertexes(const unsigned int index, const unsigned char* pData, const unsigned long long count)
	{
	}
	void Mesh::UpdateIndexes(const GraphicsQueue* pQueue,const unsigned char* pData,const unsigned long long offset, const unsigned long long count)
	{
		if (mIndexCount == 0)
			return;

		//Update host buffer
		mDevice->UpdateHostBuffer(mIndexHostBuffer, pData, count * mBytesPerIndex, offset*mBytesPerIndex);

		//Update device buffer
		mCmdList->Begin();
		mCmdList->End();
		mDevice->SubmitCommandLists(pQueue, (const CommandList**)&mCmdList,1,nullptr,mFence,nullptr,0,nullptr,0);
		mFence->Wait();
		mFence->Reset();
	}
	void Mesh::FreeIndexes()
	{
		if (mIndexHostBuffer != nullptr)
		{
			delete mIndexHostBuffer;
			mIndexHostBuffer = nullptr;
		}
		if (mIndexDeviceBuffer != nullptr)
		{
			delete mIndexDeviceBuffer;
			mIndexDeviceBuffer = nullptr;
		}
	}
}
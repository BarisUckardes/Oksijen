#pragma once
#include <Runtime/Graphics/Memory/GraphicsMemory.h>
#include <Runtime/Graphics/Buffer/GraphicsBuffer.h>
#include <Runtime/Resource/Mesh/MeshLayout.h>
#include <vector>

namespace Oksijen
{
	class RUNTIME_API Mesh final
	{
	public:
		Mesh(GraphicsMemory* pHostMemory,GraphicsMemory* pDeviceMemory,const MeshLayout& layout,const unsigned char bytesPerIndex);
		~Mesh();

		FORCEINLINE const MeshLayout& GetLayout() const noexcept { return mLayout; }
		FORCEINLINE unsigned int GetBytesPerVertex() const noexcept { return mBytesPerVertex; }
		FORCEINLINE unsigned char GetBytesPerIndex() const noexcept { return mBytesPerIndex; }
		FORCEINLINE const std::vector<GraphicsBuffer*>& GetVertexBuffers() const noexcept { return mVertexDeviceBuffers; }
		FORCEINLINE GraphicsBuffer* GetIndexBuffer() const noexcept { return mIndexDeviceBuffer; }

		FORCEINLINE unsigned long long GetVertexCount() const noexcept { return mVertexCount; }
		FORCEINLINE unsigned long long GetIndexCount() const noexcept { return mIndexCount; }

		
	private:
		const MeshLayout& mLayout;
		const unsigned char mBytesPerIndex;
		const unsigned int mBytesPerVertex;
		unsigned long long mVertexCount;
		unsigned long long mIndexCount;
		GraphicsMemory* mHostMemory;
		GraphicsMemory* mDeviceMemory;
		std::vector<GraphicsBuffer*> mVertexDeviceBuffers;
		std::vector<GraphicsBuffer*> mVertexHostBuffers;
		GraphicsBuffer* mIndexDeviceBuffer;
		GraphicsBuffer* mIndexHostBuffer;
	};
}
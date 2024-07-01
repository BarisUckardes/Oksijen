#pragma once
#include <Runtime/Core/Core.h>
#include <Runtime/Resource/Mesh/MeshVertexLayout.h>

namespace Oksijen
{
	class RUNTIME_API MeshLayout
	{
	public:
		MeshLayout(const std::vector<MeshVertexLayout>& vertexLayouts);
		~MeshLayout() = default;

		FORCEINLINE const std::vector<MeshVertexLayout>& GetVertexLayouts() const noexcept { return mVertexLayouts; }
		FORCEINLINE unsigned int GetSize() const noexcept { return mSize; }
	private:
		const std::vector<MeshVertexLayout> mVertexLayouts;
		unsigned int mSize;
	};
}
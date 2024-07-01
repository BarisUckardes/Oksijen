#include "MeshLayout.h"

namespace Oksijen
{
	MeshLayout::MeshLayout(const std::vector<MeshVertexLayout>& vertexLayouts) : mVertexLayouts(vertexLayouts)
	{
		mSize = 0;
		for (const MeshVertexLayout& vertexLayout : vertexLayouts)
		{
			mSize += vertexLayout.GetSize();
		}
	}
}
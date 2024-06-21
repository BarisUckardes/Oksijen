#pragma once
#include <Runtime/Core/Core.h>
#include <Runtime/Resource/Mesh/MeshVertexLayout.h>

namespace Oksijen
{
	struct RUNTIME_API MeshLayout
	{
		std::vector<MeshVertexLayout> VertexLayouts;
	};
}
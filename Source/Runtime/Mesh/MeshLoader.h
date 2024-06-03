#pragma once
#include <string>
#include <vector>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace Oksijen
{
	struct VertexVector2D
	{
		float X;
		float Y;
	};
	struct VertexVector3D
	{
		float X;
		float Y;
		float Z;
	};
	struct VertexVector4D
	{
		float X;
		float Y;
		float Z;
		float W;
	};

	struct RUNTIME_API MeshImportData final
	{
		std::vector<VertexVector3D> Positions;
		std::vector<VertexVector3D> Tangents;
		std::vector<VertexVector3D> Bitangents;
		std::vector<VertexVector3D> Normals;
		std::vector<std::vector<VertexVector2D>> Uvs;
		std::vector<std::vector<VertexVector4D>> Colors;
		std::vector<unsigned int> Triangles;
		unsigned long long VertexCount;
		unsigned int MaterialGroupIndex;
	};

	class MeshLoader final
	{
	public:
		static void LoadMesh(const std::string& path,const unsigned int processSteps,std::vector<MeshImportData>& importedMeshes);
	public:
		MeshLoader() = delete;
		~MeshLoader() = delete;
	};
}
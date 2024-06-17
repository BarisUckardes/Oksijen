#include "MeshLoader.h"
#include <Runtime/Core/Core.h>

namespace Oksijen
{
	void MeshLoader::LoadMesh(const std::string& path, const unsigned int processSteps, std::vector<MeshImportData>& importedMeshes, std::vector<MaterialImportData>& importedMaterials)
	{
		//Get importer
		Assimp::Importer importer;

		//Load scene
		const aiScene* pScene = importer.ReadFile(path.c_str(), processSteps);
		if (pScene == nullptr)
			return;
		
		if (pScene->HasMaterials())
		{
			for (unsigned int i = 0; i < pScene->mNumMaterials; i++)
			{
				//Get assimp material
				const aiMaterial* pMaterial = pScene->mMaterials[i];

				//Get material import data
				importedMaterials.push_back({});
				MaterialImportData& materialData = importedMaterials[importedMaterials.size() - 1];

				//Get base color
				{
					const unsigned int count = pMaterial->GetTextureCount(aiTextureType_DIFFUSE);
					if (count != 0)
					{
						aiString textureName;
						pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &textureName);

						materialData.BaseColorTextureName = textureName.C_Str();

						DEV_LOG("MeshLoader", "Found base color texture: %s", textureName.C_Str());
					}
				}

				//Get normal color
				{
					const unsigned int count = pMaterial->GetTextureCount(aiTextureType_NORMALS);
					if (count != 0)
					{
						aiString textureName;
						pMaterial->GetTexture(aiTextureType_NORMALS, 0, &textureName);

						materialData.NormalTextureName = textureName.C_Str();

						DEV_LOG("MeshLoader", "Found normal texture: %s", textureName.C_Str());
					}
				}

				//Get roughness
				{
					const unsigned int count = pMaterial->GetTextureCount(aiTextureType_DIFFUSE_ROUGHNESS);
					if (count != 0)
					{
						aiString textureName;
						pMaterial->GetTexture(aiTextureType_DIFFUSE_ROUGHNESS, 0, &textureName);

						materialData.RoughnessTextureName = textureName.C_Str();

						DEV_LOG("MeshLoader", "Found roughness texture: %s", textureName.C_Str());
					}
				}

			}
		}

		//Get meshes
		if (pScene->HasMeshes())
		{
			for (unsigned int meshIndex = 0; meshIndex < pScene->mNumMeshes; meshIndex++)
			{
				//Get assimp mesh
				const aiMesh* pMesh = pScene->mMeshes[meshIndex];

				//Get mesh import data
				importedMeshes.push_back({});
				MeshImportData& meshData = importedMeshes[importedMeshes.size() - 1];

				//Set material index
				meshData.MaterialGroupIndex = pMesh->mMaterialIndex;

				//Set vertex count
				meshData.VertexCount = pMesh->mNumVertices;

				//Set positions
				if (pMesh->HasPositions())
				{
					meshData.Positions.reserve(pMesh->mNumVertices);

					for (unsigned i = 0; i < pMesh->mNumVertices; i++)
					{
						const aiVector3D& pos = pMesh->mVertices[i];
						meshData.Positions.push_back({ pos.x,pos.y,pos.z });
					}
				}

				//Set normals
				if (pMesh->HasNormals())
				{
					meshData.Normals.reserve(pMesh->mNumVertices);

					for (unsigned int i = 0; i < pMesh->mNumVertices; i++)
					{
						const aiVector3D& normal = pMesh->mNormals[i];
						meshData.Normals.push_back({ normal.x,normal.y,normal.z });
					}
				}

				//Set tangents
				if (pMesh->HasTangentsAndBitangents())
				{
					meshData.Tangents.reserve(pMesh->mNumVertices);
					meshData.Bitangents.reserve(pMesh->mNumVertices);

					for (unsigned int i = 0; i < pMesh->mNumVertices; i++)
					{
						const aiVector3D& tangent = pMesh->mTangents[i];
						meshData.Tangents.push_back({ tangent.x,tangent.y,tangent.z });

						const aiVector3D& bitangent = pMesh->mBitangents[i];
						meshData.Bitangents.push_back({ bitangent.x,bitangent.y,bitangent.z });
					}
				}

				//Set colors
				if (pMesh->GetNumColorChannels() > 0)
				{
					for (unsigned int colorChannelIndex = 0; colorChannelIndex < pMesh->GetNumColorChannels(); colorChannelIndex++)
					{
						//Get new color channle buffer
						meshData.Colors.push_back({});
						std::vector<VertexVector4D>& colorChannelBuffer = meshData.Colors[meshData.Colors.size() - 1];

						colorChannelBuffer.reserve(pMesh->mNumVertices);

						for (unsigned int i = 0; i < pMesh->mNumVertices; i++)
						{
							const aiColor4D& color = pMesh->mColors[colorChannelIndex][i];
							colorChannelBuffer.push_back({ color.r,color.g,color.b,color.a });
						}
					}
				}

				//Set uvs
				if (pMesh->GetNumUVChannels() > 0)
				{
					for (unsigned int uvChannelIndex = 0; uvChannelIndex < pMesh->GetNumUVChannels(); uvChannelIndex++)
					{
						//Get new uv channel buffer
						meshData.Uvs.push_back({});
						std::vector<VertexVector2D>& uvChannelBuffer = meshData.Uvs[meshData.Uvs.size() - 1];

						uvChannelBuffer.reserve(pMesh->mNumVertices);

						for (unsigned int i = 0; i < pMesh->mNumVertices; i++)
						{
							const aiVector3D& uv = pMesh->mTextureCoords[uvChannelIndex][i];
							uvChannelBuffer.push_back({ uv.x,uv.y });
						}
					}
				}

				//Set triangles
				if (pMesh->HasFaces())
				{
					meshData.Triangles.reserve(pMesh->mNumFaces * 3);
					for (unsigned int i = 0; i < pMesh->mNumFaces; i++)
					{
						const aiFace& face = pMesh->mFaces[i];
						meshData.Triangles.push_back(face.mIndices[0]);
						meshData.Triangles.push_back(face.mIndices[1]);
						meshData.Triangles.push_back(face.mIndices[2]);
					}
				}

			}
		}

		
	}
}
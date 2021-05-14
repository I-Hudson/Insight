#pragma once
#include "Engine/Graphics/Model/Model.h"

struct aiNode;
struct aiMesh;
struct aiMaterial;
enum aiTextureType;
struct aiScene;

namespace Insight::Graphics::ModelLoading
{
	struct AssimpLoader
	{
		static void LoadFromFile(Model& model, const std::string& filePath);
		static void ProcessNode(Mesh& mesh, aiNode* aiNode, const aiScene* aiScene, const std::string& directory);
		static void ProcessMesh(Mesh& mesh, aiMesh* aiMesh, const aiScene* aiScene, const std::string& directory);
		static MeshTextures LoadMateials(aiMaterial* aiMaterial, aiTextureType aiType, const std::string& typeName, const std::string& directory);
	};
}
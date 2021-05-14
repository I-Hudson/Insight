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
		static void ProcessNode();
		static void ProcessMesh();
	};
}
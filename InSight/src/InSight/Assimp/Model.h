#pragma once

#include "Insight/Core.h"
#include "Insight/Assimp/Mesh.h"

#include "Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include <string>
#include <vector>

class IS_API Model
{
public:
	Model() { LoadMesh("./models/Survival_BackPack_2/backpack.obj"); }
	Model(const std::string& filePath);
	~Model();

	Mesh* GetSubMesh(int index);
	unsigned int GetSubMeshCount() { return static_cast<unsigned int>(m_meshes.size()); }

	const std::string& GetName() const;

private:
	void LoadMesh(const std::string& filePath);
	void ProcessNode(aiNode* node, const aiScene* scene);
	Mesh* ProcessMesh(aiMesh* mesh, const aiScene* scene);
	std::vector<Texture> LoadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);

	unsigned int TextureFromFile(const std::string& path, const std::string& directory, bool gamma = false);

	std::vector<Mesh*> m_meshes;
	std::vector<Texture> m_texturesLoaded;
	std::string m_directory;
	std::string m_modelName;
};


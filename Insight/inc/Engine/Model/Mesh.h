#pragma once
#include "Engine/Core/Core.h"
#include "Engine/Core/Object.h"
#include "SubMesh.h"
#include "Engine/Graphics/Material.h"
#include "Engine/Module/GraphicsModule.h"

class Model;
struct aiNode;
struct aiMesh;
struct aiMaterial;
enum aiTextureType;
struct aiScene;

using LoadedTextures = std::vector<std::pair<std::string, Render::Texture*>>;
class MeshComponent;
using MeshMaterialUpdateFunc = const std::function<void(MeshComponent* meshCom, Material* materai, MaterialBlockData& materialBlockData)>&;

class Mesh : public Object
{
public:
	Mesh();
	~Mesh();

	void SetVertices(const std::vector<Vertex>& vertices, const u32& submeshIndex = 0);
	void SetIndices(const std::vector<u32>& indices, const u32& submeshIndex = 0);
	void Rebuild();
	SubMesh& GetSubMesh(u32 index) { return *m_subMeshes.at(index); }

	// Getters for all the values and setters.
	const std::string& GetMeshName() const { return m_meshName; }
	const u32 GetMeshSubCount() { return static_cast<u32>(m_subMeshes.size()); }

private:
	void LoadSubMeshes(const std::string& filePath, Model& model);
	void ProcessNode(aiNode* node, const aiScene* scene);
	SubMesh* ProcessMesh(aiMesh* mesh, const aiScene* scene);

	LoadedTextures LoadMaterialTextures(aiMaterial* mat, aiTextureType type, const std::string& typeName);

private:
	std::vector<SubMesh*> m_subMeshes;
	Model* m_model;

	std::string m_meshName;
	std::string m_directory;
	friend class Model;
};

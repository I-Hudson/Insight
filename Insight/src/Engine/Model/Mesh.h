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

using LoadedTextures = std::vector<std::pair<std::string, SharedPtr<Render::Texture>>>;
	namespace Render
	{
		class Texture;
	}
class MeshComponent;
using MeshMaterialUpdateFunc = const std::function<void(MeshComponent* meshCom, SharedPtr<Material> materai, MaterialBlockData& materialBlockData)>&;

class Mesh : public Object
{
public:
	~Mesh();

	void SetVertices(const std::vector<Vertex>& vertices, const U32& submeshIndex = 0);
	void SetIndices(const std::vector<U32>& indices, const U32& submeshIndex = 0);
	void Rebuild();

	// Getters for all the values and setters.
	const std::string& GetMeshName() const { return m_meshName; }
	const U32& GetMeshSubCount() { return static_cast<U32>(m_subMeshes.size()); }

	void Draw(VkCommandBuffer cmd);
	void Draw(VkCommandBuffer cmd, const std::vector<WeakPtr<Material>>& materials, const std::vector<MaterialBlockData>& materialBlockDatas, MeshMaterialUpdateFunc materialUpdateFunc, MeshComponent* meshCompoennt);

private:
	void LoadSubMeshes(const std::string& filePath, Model& model);
	void ProcessNode(aiNode* node, const aiScene* scene);
	SharedPtr<SubMesh> ProcessMesh(aiMesh* mesh, const aiScene* scene);

	LoadedTextures LoadMaterialTextures(aiMaterial* mat, aiTextureType type, const std::string& typeName);

private:
	std::vector<SharedPtr<SubMesh>> m_subMeshes;
	Model* m_model;

	std::string m_meshName;
	std::string m_directory;
	friend class Model;
};

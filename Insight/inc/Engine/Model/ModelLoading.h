#pragma once
#include "Engine/Model/Model.h"

#include "assimp/Quaternion.h"
#include "assimp/vector3.h"
#include "assimp/matrix4x4.h"

struct aiNode;
struct aiMesh;
struct aiMaterial;
enum aiTextureType;
struct aiScene;

namespace Insight::ModelLoading
{
	struct AssimpLoader
	{
		static void LoadFromFile(Model& model, const std::string& filePath);
		static void ProcessNode(Mesh& mesh, aiNode* aiNode, const aiScene* aiScene, const std::string& directory);
		static SubMesh ProcessMesh(Mesh& mesh, aiMesh* aiMesh, const aiScene* aiScene, const std::string& directory);
		static MeshTextures LoadMateials(aiMaterial* aiMaterial, aiTextureType aiType, const std::string& typeName, const std::string& directory);
		
		static void ExtractBoneInfo(Mesh& mesh, aiMesh* aiMesh, const aiScene* aiScene);
		static void LoadAnimations(Model& model, const aiScene* aiScene);

		static glm::mat4 AssimpToGLM(const aiMatrix4x4& from);
		static aiMatrix4x4 GLMToAssimp(const glm::mat4& from);
		static glm::vec3 GetGLMVec(const aiVector3D& vec);
		static glm::quat GetGLMQuat(const aiQuaternion& pOrientation);

	private:
		static void SetVertexBoneData(Vertex& vertex, int boneID, float weight);
	};
}
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

namespace tinygltf
{
	class Node;
	struct Mesh;
	class Model;
	struct Material;
	struct Scene;
	struct Skin;
}

namespace Insight::ModelLoading
{
	struct ModelLoader
	{
		static void LoadFromFile(Model& model, const std::string& filePath);
	};

	struct AssimpLoader
	{
		static void LoadFromFile(Model& model, const std::string& filePath);
		static void ProcessNode(Mesh& mesh, aiNode* aiNode, const aiScene* aiScene, const std::string& directory);
		static SubMesh ProcessMesh(Mesh& mesh, aiMesh* aiMesh, const aiScene* aiScene, const std::string& directory);
		static MeshTextures LoadMateials(aiMaterial* aiMaterial, aiTextureType aiType, const std::string& typeName, const std::string& directory);
		
		static void SetVertexBoneDataToDefault(Vertex& vertex);
		static void ExtractBoneWeightFromVertices(std::vector<Vertex>& vertices, Mesh& mesh, aiMesh* aiMesh, const aiScene* aiScene);
		static void LoadAnimations(Model& model, const aiScene* aiScene);

		static glm::mat4 AssimpToGLM(const aiMatrix4x4& from);
		static aiMatrix4x4 GLMToAssimp(const glm::mat4& from);
		static glm::vec3 GetGLMVec(const aiVector3D& vec);
		static glm::quat GetGLMQuat(const aiQuaternion& pOrientation);

	private:
		static void SetVertexBoneData(Vertex& vertex, int boneID, float weight);
	};

	struct GltfLoader
	{
		static void LoadFromFile(Model& model, const std::string& filePath);
		static void ProcessNode(Mesh& mesh, const tinygltf::Node& gltfNode, const tinygltf::Model& gltdModel, const std::string& directory);
		static void ProcessMesh(Mesh& mesh, const tinygltf::Mesh& gltfMesh, const tinygltf::Model& gltfModel, const std::string& directory);
		static void LoadSkeleton(Mesh& mesh, const tinygltf::Skin& gltfSkin, const tinygltf::Model& gltfModel);
		static MeshTextures LoadMateials(tinygltf::Material& gltfMaterial, const std::string& typeName, const std::string& directory);

		static glm::mat4 GetNodeMatrix(const tinygltf::Node& gltfNode);
		static void SetVertexBoneDataToDefault(Vertex& vertex);
		static void ExtractBoneWeightFromVertices(std::vector<Vertex>& vertices, Mesh& mesh, const tinygltf::Mesh& gltfMesh, const tinygltf::Model& gltfModel);
		static void LoadAnimations(Model& model, const tinygltf::Model& gltfModel);

		static glm::quat GetGlmQuat(glm::vec4 vec);
	};
}
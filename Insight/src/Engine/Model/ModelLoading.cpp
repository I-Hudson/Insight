#include "ispch.h"
#include "Engine/Model/ModelLoading.h"
#include "Engine/Model/Bone.h"
#include "glm/gtc/type_ptr.hpp"
#include <filesystem>
#include "Engine/FileSystem/FileSystem.h"
#include "Engine/Graphics/Graphics.h"

#include "Engine/Module/GraphicsModule.h"
#define STB_IMAGE_IMPLEMENTATION
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "Assimp/mesh.h"
#include "assimp/postprocess.h"
#include "stb_image.h"

namespace Insight::ModelLoading
{
	/// <summary>
	/// AssimpLoader
	/// </summary>
	/// <param name="model"></param>
	/// <param name="filePath"></param>
	void AssimpLoader::LoadFromFile(Model& model, const std::string& filePath)
	{
		IS_PROFILE_FUNCTION();
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(filePath, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals | aiProcess_OptimizeGraph | aiProcess_OptimizeMeshes);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			IS_CORE_ERROR("Assimp model load: {0}", importer.GetErrorString());
			return;
		}
		// retrieve the directory path of the filepath
		std::string fileDirectory = std::move(FileSystem::FileSystemManager::WindowsToUinxFilePath(filePath));
		fileDirectory = std::move(fileDirectory.substr(0, fileDirectory.find_last_of('/')));
		model.m_mesh.m_meshName = scene->mRootNode->mName.C_Str();
		ProcessNode(model.m_mesh, scene->mRootNode, scene, fileDirectory);


		for (auto& subMesh : model.m_mesh.m_subMeshes)
		{
			model.m_mesh.m_dimensions.Min.x = glm::min(subMesh.m_dimensions.Min.x, model.m_mesh.m_dimensions.Min.x);
			model.m_mesh.m_dimensions.Min.y = glm::min(subMesh.m_dimensions.Min.y, model.m_mesh.m_dimensions.Min.y);
			model.m_mesh.m_dimensions.Min.z = glm::min(subMesh.m_dimensions.Min.z, model.m_mesh.m_dimensions.Min.z);
			model.m_mesh.m_dimensions.Max.x = glm::max(subMesh.m_dimensions.Max.x, model.m_mesh.m_dimensions.Max.x);
			model.m_mesh.m_dimensions.Max.y = glm::max(subMesh.m_dimensions.Max.y, model.m_mesh.m_dimensions.Max.y);
			model.m_mesh.m_dimensions.Max.z = glm::max(subMesh.m_dimensions.Max.z, model.m_mesh.m_dimensions.Max.z);
			model.m_mesh.m_dimensions.Center += subMesh.m_dimensions.Center;
		}

		// Work out the mesh's centre, size and radius. This needs to be done last 
		// as we need all the sub meshes information for this.
		glm::vec3& meshCenter = model.m_mesh.m_dimensions.Center;
		u32 numOfSubMeshes = (u32)model.m_mesh.m_subMeshes.size();
		meshCenter = glm::vec3(meshCenter.x / numOfSubMeshes, meshCenter.y / numOfSubMeshes, meshCenter.z / numOfSubMeshes);

		model.m_mesh.m_dimensions.Size = model.m_mesh.m_dimensions.Max - model.m_mesh.m_dimensions.Min;
		if (::Graphics::IsVulkan())
		{
			model.m_mesh.m_dimensions.Size.y *= -1;
		}
		model.m_mesh.m_dimensions.Radius = glm::distance(meshCenter, model.m_mesh.m_dimensions.Max);

		LoadAnimations(model, scene);
	}

	void AssimpLoader::ProcessNode(Mesh& mesh, aiNode* aiNode, const aiScene* aiScene, const std::string& directory)
	{
		IS_PROFILE_FUNCTION();

		// process all the node's meshes (if any)
		for (u32 i = 0; i < aiNode->mNumMeshes; i++)
		{
			aiMesh* aiMesh = aiScene->mMeshes[aiNode->mMeshes[i]];
			mesh.m_subMeshes.push_back(ProcessMesh(mesh, aiMesh, aiScene, directory));
		}
		// then do the same for each of its children
		for (u32 i = 0; i < aiNode->mNumChildren; i++)
		{
			ProcessNode(mesh, aiNode->mChildren[i], aiScene, directory);
		}
	}

	SubMesh AssimpLoader::ProcessMesh(Mesh& mesh, aiMesh* aiMesh, const aiScene* aiScene, const std::string& directory)
	{
		IS_PROFILE_FUNCTION();

		std::vector<Vertex> vertices;
		std::vector<u32> indices;

		u32 vertexStart = mesh.m_vertices.size();
		u32 indicesStart = mesh.m_indices.size();

		MeshDimensions dimensions;
		glm::vec3 centre = glm::vec3(0);

		for (unsigned int i = 0; i < aiMesh->mNumVertices; ++i)
		{
			Vertex vertex;
		
			SetVertexBoneDataToDefault(vertex);
			
			// process vertex positions, normals and texture coordinates
			glm::vec3 position;
			position.x = aiMesh->mVertices[i].x;
			//if (Module::GraphicsModule::Instance()->IsVulkan())
			//{
			//	position.y = aiMesh->mVertices[i].y *= -1;
			//}
			//else
			{
				position.y = aiMesh->mVertices[i].y;
			}
			position.z = aiMesh->mVertices[i].z;
			vertex.Position = position;

			dimensions.Min.x = glm::min(position.x, dimensions.Min.x);
			dimensions.Min.y = glm::min(position.y, dimensions.Min.y);
			dimensions.Min.z = glm::min(position.z, dimensions.Min.z);
			dimensions.Max.x = glm::max(position.x, dimensions.Max.x);
			dimensions.Max.y = glm::max(position.y, dimensions.Max.y);
			dimensions.Max.z = glm::max(position.z, dimensions.Max.z);
			centre += position;

			glm::vec4 colour = glm::vec4();
			colour.r = 1.0f; // static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
			colour.g = 1.0f; // static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
			colour.b = 1.0f; // static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
			colour.a = 1.0f; // static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
			vertex.Colour = colour;

			glm::vec3 normal;
			if (aiMesh->mNormals != nullptr)
			{
				normal.x = aiMesh->mNormals[i].x;
				if (Module::GraphicsModule::Instance()->IsVulkan())
				{
					normal.y = aiMesh->mNormals[i].y *= -1;
				}
				else
				{
					normal.y = aiMesh->mNormals[i].y;
				}
				normal.z = aiMesh->mNormals[i].z;
			}
			vertex.Normal = normal;

			glm::vec2 uv;
			if (aiMesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
			{
				uv.x = aiMesh->mTextureCoords[0][i].x;
				uv.y = aiMesh->mTextureCoords[0][i].y;
			}
			else
			{
				uv.x = 0.0f;
				uv.y = 0.0f;
			}
			vertex.UV1 = uv;
#ifdef IS_MESH_BATCHING_EXT
			vertex.VIndex = (u32)mesh.m_subMeshes.size();
#endif
			vertices.push_back(vertex);
		}

		if (::Graphics::IsVulkan())
		{
			std::swap(dimensions.Min.y, dimensions.Max.y);
			dimensions.Min.y *= -1;
			dimensions.Max.y *= -1;
		}

		dimensions.Center = glm::vec3(centre.x / aiMesh->mNumVertices, centre.y / aiMesh->mNumVertices,centre.z / aiMesh->mNumVertices);
		dimensions.Size = dimensions.Max - dimensions.Min;

		for (unsigned int i = 0; i < aiMesh->mNumVertices; ++i)
		{
			glm::vec3 position;
			position.x = aiMesh->mVertices[i].x;
			if (Module::GraphicsModule::Instance()->IsVulkan())
			{
				position.y = aiMesh->mVertices[i].y *= -1;
			}
			else
			{
				position.y = aiMesh->mVertices[i].y;
			}
			position.z = aiMesh->mVertices[i].z;
			dimensions.Radius = glm::max(glm::distance(dimensions.Center, position), dimensions.Radius);
		}

		for (unsigned int i = 0; i < aiMesh->mNumFaces; ++i)
		{
			aiFace face = aiMesh->mFaces[i];
			for (unsigned int j = 0; j < face.mNumIndices; ++j)
			{
				//mesh.m_indices.push_back(face.mIndices[j]);
				indices.push_back(face.mIndices[j]);
			}
		}

		//SubMesh subMesh = SubMesh(0, vertices.size(), 0, indices.size(), vertices, indices);
		SubMesh subMesh = SubMesh(vertexStart, (u32)vertices.size(), indicesStart, (u32)indices.size(), mesh.m_vertexBuffer, mesh.m_indexBuffer);
		subMesh.m_dimensions = dimensions;

		if (aiMesh->mMaterialIndex >= 0)
		{
			aiMaterial* aiMaterial = aiScene->mMaterials[aiMesh->mMaterialIndex];
			auto materials = LoadMateials(aiMaterial, aiTextureType_DIFFUSE, "texture_diffuse", directory);
			subMesh.m_textures = materials;
			subMesh.m_textureStrings[MaterialTextureType::Diffuse] = materials["texture_diffuse"];
		}

		ExtractBoneWeightFromVertices(vertices, mesh, aiMesh, aiScene);
		mesh.m_vertices.insert(mesh.m_vertices.end(), vertices.begin(), vertices.end());
		mesh.m_indices.insert(mesh.m_indices.end(), indices.begin(), indices.end());
		subMesh.m_isSkinnedMesh = mesh.m_isSkinnedMesh;

		return subMesh;
	}

	MeshTextures AssimpLoader::LoadMateials(aiMaterial* aiMaterial, aiTextureType aiType, const std::string& typeName, const std::string& directory)
	{
		IS_PROFILE_FUNCTION();

		MeshTextures textures;
		for (unsigned int i = 0; i < aiMaterial->GetTextureCount(aiType); ++i)
		{
			aiString str;
			aiMaterial->GetTexture(aiType, i, &str);
			bool skip = false;
			std::string formatedString = str.C_Str();
			if (formatedString.find(":") != std::string::npos)
			{
				formatedString = formatedString.substr(formatedString.find_last_of('\\') + 1);
			}

			//for (unsigned int j = 0; j < loadedTextures.size(); ++j)
			//{
			//	if (std::strcmp(m_loadedTextures[j].GetFilePath().c_str(), str.C_Str()) == 0)
			//	{
			//		textures.push_back(m_loadedTextures[j]);
			//		skip = true;
			//		break;
			//	}
			//}
			if (!skip)
			{
				//if texture hasn't been loaded already, load it
				//Texture* texture = Texture::New();
				//texture->Init(m_directory + "/" + str.C_Str());
				//textures.emplace_back(typeName, texture);
				//m_loadedTextures.push_back(texture); // add to loaded textures
				textures.insert({ typeName, directory + '/' + formatedString });
			}
		}
		return textures;
	}

	void AssimpLoader::SetVertexBoneDataToDefault(Vertex& vertex)
	{
		vertex.JointIndices = glm::ivec4(-1, -1, -1, -1);
		vertex.JointWeight = glm::vec4(0, 0, 0, 0);
	}

	void AssimpLoader::ExtractBoneWeightFromVertices(std::vector<Vertex>& vertices, Mesh& mesh, aiMesh* aiMesh, const aiScene* aiScene)
	{
		if (mesh.GetSkeleton().GetBoneCount() == 0)
		{
			mesh.m_skeleton = Animation::Skeleton(aiMesh->mNumBones, glm::inverse(AssimpToGLM(aiScene->mRootNode->mTransformation)));
		}

		for (u32 boneIndex = 0; boneIndex < aiMesh->mNumBones; ++boneIndex)
		{
			mesh.m_isSkinnedMesh = true;
			int boneID = -1;
			std::string boneName = aiMesh->mBones[boneIndex]->mName.C_Str();
			if (!mesh.m_skeleton.HasBone(boneName))
			{
				mesh.m_skeleton.AddBone(boneName, AssimpToGLM(aiMesh->mBones[boneIndex]->mOffsetMatrix));
				boneID = mesh.m_skeleton.GetBone(boneName).GetBoneId();
			}
			else
			{
				boneID = mesh.m_skeleton.GetBone(boneName).GetBoneId();
			}

			ASSERT(boneID != -1);
			auto weights = aiMesh->mBones[boneIndex]->mWeights;
			int numWeights = aiMesh->mBones[boneIndex]->mNumWeights;
			for (u32 weightIndex = 0; weightIndex < numWeights; ++weightIndex)
			{
				u32 vertexId = weights[weightIndex].mVertexId;
				float weight = weights[weightIndex].mWeight;
				ASSERT(vertexId <= vertices.size());
				SetVertexBoneData(vertices.at(vertexId), boneID, weight);
			}
		}
	}

	void AssimpLoader::LoadAnimations(Model& model, const aiScene* aiScene)
	{
		if (model.GetMesh().m_skeleton.GetBoneCount() > 0)
		{
			model.GetMesh().m_skeleton.PopulateBoneData(aiScene);
			model.GetMesh().m_skeleton.GetRootBone().CalcInverseBindTransform(glm::mat4(1));
		}

		std::vector<Animation::Animation> animations;
		for (u32 i= 0; i <  aiScene->mNumAnimations; ++i)
		{
			animations.push_back(Animation::Animation(aiScene, i, &model));
		}
		model.m_mesh.m_animations = animations;
	}

	glm::mat4 AssimpLoader::AssimpToGLM(const aiMatrix4x4& from)
	{
		return glm::mat4(
			(double)from.a1, (double)from.b1, (double)from.c1, (double)from.d1,
			(double)from.a2, (double)from.b2, (double)from.c2, (double)from.d2,
			(double)from.a3, (double)from.b3, (double)from.c3, (double)from.d3,
			(double)from.a4, (double)from.b4, (double)from.c4, (double)from.d4);
	}

	aiMatrix4x4 AssimpLoader::GLMToAssimp(const glm::mat4& from)
	{
		return aiMatrix4x4(from[0][0], from[1][0], from[2][0], from[3][0],
						   from[0][1], from[1][1], from[2][1], from[3][1],
						   from[0][2], from[1][2], from[2][2], from[3][2],
						   from[0][3], from[1][3], from[2][3], from[3][3]
		);
	}

	glm::vec3 AssimpLoader::GetGLMVec(const aiVector3D& vec)
	{
		return glm::vec3(vec.x, vec.y, vec.z); 
	}

	glm::quat AssimpLoader::GetGLMQuat(const aiQuaternion& pOrientation)
	{
		return glm::quat(pOrientation.w, pOrientation.x, pOrientation.y, pOrientation.z);
	}

	void AssimpLoader::SetVertexBoneData(Vertex& vertex, int boneID, float weight)
	{
		for (u32 i = 0; i < 4; ++i)
		{
			if (vertex.JointIndices[i] < 0)
			{
				vertex.JointIndices[i] = boneID;
				vertex.JointWeight[i] = weight;
				break;
			}
		}
	}
}
#include "ispch.h"
#include "Engine/Graphics/Model/ModelLoading.h"
#include "glm/gtc/type_ptr.hpp"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "Assimp/mesh.h"
#include "assimp/postprocess.h"

namespace Insight::Graphics::ModelLoading
{
	/// <summary>
	/// GLTFModelLoader
	/// </summary>
	/// <param name="model"></param>
	/// <param name="filePath"></param>
	void GLTFModelLoader::LoadFromFile(Model& model, const std::string& filePath)
	{
		tinygltf::Model gltfModel;
		tinygltf::TinyGLTF gltfContext;

		u64 pos = filePath.find_last_of('/');
		model.m_path = filePath.substr(0, pos);

		std::string err, warning;
		if (!gltfContext.LoadASCIIFromFile(&gltfModel, &err, &warning, filePath))
		{
			IS_ERROR("[GLTFModelLoader::LoadFromFile] Unable to load: {0}", filePath);
			return;
		}
		IS_CORE_INFO("[GLTFModelLoader::LoadFromFile] Loading file: {0}.", filePath);

		const tinygltf::Scene& scene = gltfModel.scenes.at(gltfModel.defaultScene > -1 ? gltfModel.defaultScene : 0);
		for (u64 i = 0; i < scene.nodes.size(); ++i)
		{
			const tinygltf::Node node = gltfModel.nodes.at(scene.nodes.at(i));

			model.m_meshes.push_back(Mesh());
			Mesh& mesh = model.m_meshes.back();
			LoadNode(mesh, node, gltfModel);
		}
	}

	void GLTFModelLoader::LoadNode(Mesh& mesh, const tinygltf::Node& node, const tinygltf::Model& gltfModel)
	{
		glm::vec3 translation = glm::vec3(0.0f);
		if (node.translation.size() == 3)
		{
			translation = glm::make_vec3(node.translation.data());
		}
		glm::quat rotation = glm::quat();
		if (node.rotation.size() == 4)
		{
			rotation = glm::make_quat(node.rotation.data());
		}
		glm::vec3 scale = glm::vec3(1.0f);
		if (node.scale.size() == 3)
		{
			scale = glm::make_vec3(node.scale.data());
		}
		glm::mat4 matrix = glm::mat4(1.0f);
		if (node.matrix.size() == 3)
		{
			matrix = glm::make_mat4x4(node.matrix.data());
		}

		if (node.children.size() > 0)
		{
			for (u64 i = 0; i < node.children.size(); ++i)
			{
				LoadNode(mesh, gltfModel.nodes.at(node.children.at(i)), gltfModel);
			}
		}

		// Mesh data from this node.
		if (node.mesh > -1)
		{
			const tinygltf::Mesh gltfMesh = gltfModel.meshes.at(node.mesh);
			for (u64 i = 0; i < gltfMesh.primitives.size(); ++i)
			{
				const tinygltf::Primitive& primitive = gltfMesh.primitives.at(i);
				if (primitive.indices < 0)
				{
					continue;
				}

				u32 vertexStart = static_cast<u32>(mesh.m_vertices.size());
				u32 indexStart = static_cast<u32>(mesh.m_indices.size());
				u32 vertexCount = 0;
				u32 indexCount = 0;
				glm::vec3 posMin = { };
				glm::vec3 posMax = { };

				{
					// Vertices
					const float*	bufferPos = nullptr;
					const float*	bufferNormals = nullptr;
					const float*	bufferTexCoords = nullptr;
					const float*	bufferColors = nullptr;
					const float*	bufferTangents = nullptr;
					u32				numColorComponents;
					const u16* bufferJoints = nullptr;
					const float*	bufferWeights = nullptr;

					// Position attribute required.
					ASSERT(primitive.attributes.find("POSITION") != primitive.attributes.end());

					const tinygltf::Accessor& posAccessor = gltfModel.accessors.at(primitive.attributes.find("POSITION")->second);
					const tinygltf::BufferView& posView = gltfModel.bufferViews.at(posAccessor.bufferView);
					bufferPos = reinterpret_cast<const float*>(&(gltfModel.buffers.at(posView.buffer).data.at(posAccessor.byteOffset + posView.byteOffset)));
					posMin = glm::vec3(posAccessor.minValues.at(0), posAccessor.minValues.at(1), posAccessor.minValues.at(2));
					posMax = glm::vec3(posAccessor.maxValues.at(0), posAccessor.maxValues.at(1), posAccessor.maxValues.at(2));

					if (primitive.attributes.find("NORMAL") != primitive.attributes.end()) 
					{
						const tinygltf::Accessor& normAccessor = gltfModel.accessors.at(primitive.attributes.find("NORMAL")->second);
						const tinygltf::BufferView& normView = gltfModel.bufferViews.at(normAccessor.bufferView);
						bufferNormals = reinterpret_cast<const float*>(&(gltfModel.buffers.at(normView.buffer).data.at(normAccessor.byteOffset + normView.byteOffset)));
					}


					if (primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end()) {
						const tinygltf::Accessor& uvAccessor = gltfModel.accessors.at(primitive.attributes.find("TEXCOORD_0")->second);
						const tinygltf::BufferView& uvView = gltfModel.bufferViews.at(uvAccessor.bufferView);
						bufferTexCoords = reinterpret_cast<const float*>(&(gltfModel.buffers[uvView.buffer].data[uvAccessor.byteOffset + uvView.byteOffset]));
					}

					if (primitive.attributes.find("COLOR_0") != primitive.attributes.end())
					{
						const tinygltf::Accessor& colorAccessor = gltfModel.accessors.at(primitive.attributes.find("COLOR_0")->second);
						const tinygltf::BufferView& colorView = gltfModel.bufferViews.at(colorAccessor.bufferView);
						// Color buffer are either of type vec3 or vec4
						numColorComponents = colorAccessor.type == TINYGLTF_PARAMETER_TYPE_FLOAT_VEC3 ? 3 : 4;
						bufferColors = reinterpret_cast<const float*>(&(gltfModel.buffers.at(colorView.buffer).data.at(colorAccessor.byteOffset + colorView.byteOffset)));
					}

					if (primitive.attributes.find("TANGENT") != primitive.attributes.end())
					{
						const tinygltf::Accessor& tangentAccessor = gltfModel.accessors.at(primitive.attributes.find("TANGENT")->second);
						const tinygltf::BufferView& tangentView = gltfModel.bufferViews.at(tangentAccessor.bufferView);
						bufferTangents = reinterpret_cast<const float*>(&(gltfModel.buffers.at(tangentView.buffer).data.at(tangentAccessor.byteOffset + tangentView.byteOffset)));
					}

					vertexCount = static_cast<u32>(posAccessor.count);

					for (u64 v = 0; v < posAccessor.count; ++v)
					{
						Vertex vert = { };
						vert.Position = glm::vec4(glm::make_vec3(&bufferPos[v * 3]), 1.0f);
						vert.Normal = glm::normalize(glm::vec3(bufferNormals ? glm::make_vec3(&bufferNormals[v * 3]) : glm::vec3(0.0f)));
						vert.UV1 = bufferTexCoords ? glm::make_vec2(&bufferTexCoords[v * 2]) : glm::vec3(0.0f);
						if (bufferColors) 
						{
							switch (numColorComponents)
							{
								case 3:
									vert.Colour = glm::vec4(glm::make_vec3(&bufferColors[v * 3]), 1.0f);
								case 4:
									vert.Colour = glm::make_vec4(&bufferColors[v * 4]);
							}
						}
						else
						{
							vert.Colour = glm::vec4(1.0f);
						}
						//vert.tangent = bufferTangents ? glm::vec4(glm::make_vec4(&bufferTangents[v * 4])) : glm::vec4(0.0f);
						//vert.joint0 = hasSkin ? glm::vec4(glm::make_vec4(&bufferJoints[v * 4])) : glm::vec4(0.0f);
						//vert.weight0 = hasSkin ? glm::make_vec4(&bufferWeights[v * 4]) : glm::vec4(0.0f);
						mesh.m_vertices.push_back(vert);
					}
				}

				{
					// Indices
					const tinygltf::Accessor& accessor = gltfModel.accessors.at(primitive.indices);
					const tinygltf::BufferView& bufferView = gltfModel.bufferViews.at(accessor.bufferView);
					const tinygltf::Buffer& buffer = gltfModel.buffers.at(bufferView.buffer);

					indexCount = static_cast<u32>(accessor.count);

					switch (accessor.componentType) {
						case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT:
						{
							std::vector<u32> buf = std::vector<u32>(accessor.count);
							Platform::MemCopy(buf.data(), &buffer.data.at(accessor.byteOffset + bufferView.byteOffset), accessor.count * sizeof(u32));
							for (size_t index = 0; index < accessor.count; index++) 
							{
								mesh.m_indices.push_back(buf[index] + vertexStart);
							}
							break;
						}
						case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: 
						{
							std::vector<u16> buf = std::vector<u16>(accessor.count);
							Platform::MemCopy(buf.data(), &buffer.data.at(accessor.byteOffset + bufferView.byteOffset), accessor.count * sizeof(u16));
							for (size_t index = 0; index < accessor.count; index++) 
							{
								mesh.m_indices.push_back(buf[index] + vertexStart);
							}
							break;
						}
						case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE: 
						{
							std::vector<u8> buf = std::vector<u8>(accessor.count);
							Platform::MemCopy(buf.data(), &buffer.data.at(accessor.byteOffset + bufferView.byteOffset), accessor.count * sizeof(u8));
							for (size_t index = 0; index < accessor.count; index++)
							{
								mesh.m_indices.push_back(buf[index] + vertexStart);
							}
							break;
						}
						default:
							IS_CORE_ERROR("Index component type {0} not supported!", accessor.componentType);
							return;
					}
				}

				SubMesh subMesh = SubMesh(vertexStart, vertexCount, indexStart, indexCount);
				subMesh.SetDimensions(posMin, posMax);
				mesh.m_subMeshes.push_back(subMesh);
			}
		}
	}

	/// <summary>
	/// FBXModelLoader
	/// </summary>
	/// <param name="model"></param>
	/// <param name="filePath"></param>
	void FBXModelLoader::LoadFromFile(Model& model, const std::string& filePath)
	{

	}

	/// <summary>
	/// OBJModelLoader
	/// </summary>
	/// <param name="model"></param>
	/// <param name="filePath"></param>
	void OBJModelLoader::LoadFromFile(Model& model, const std::string& filePath)
	{

	}
}
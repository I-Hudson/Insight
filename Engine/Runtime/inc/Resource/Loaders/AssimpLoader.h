#pragma once

#include "Core/TypeAlias.h"

#include "Graphics/Mesh.h"
#include "Graphics/RHI/RHI_Texture.h"

#include <assimp/postprocess.h>
#include <string>

struct aiNode;
struct aiScene;
struct aiMesh;
struct aiMaterial;
enum aiTextureType;

namespace Insight
{
	namespace Runtime
	{
		class Model;
		class Mesh;
		class Texture2D;

		/// @brief Utility class to help with anything related to loading from assimp.
		class AssimpLoader
		{
		public:
			static const uint32_t Default_Model_Importer_Flags =
				// Switch to engine conventions
				//aiProcess_FlipUVs                   |	/// DirectX style.
				aiProcess_MakeLeftHanded            |	
				// Validate and clean up
				aiProcess_ValidateDataStructure     |	/// Validates the imported scene data structure. This makes sure that all indices are valid, all animations and bones are linked correctly, all material references are correct
				aiProcess_FindDegenerates           |	/// Convert degenerate primitives to proper lines or points.
				aiProcess_FindInvalidData           |	/// This step searches all meshes for invalid data, such as zeroed normal vectors or invalid UV coords and removes / fixes them
				aiProcess_RemoveRedundantMaterials  |	/// Searches for redundant/unreferenced materials and removes them
				aiProcess_Triangulate               |	/// Triangulates all faces of all meshes
				aiProcess_JoinIdenticalVertices     |	/// Triangulates all faces of all meshes
				aiProcess_SortByPType               |	/// Splits meshes with more than one primitive type in homogeneous sub-meshes.
				aiProcess_FindInstances             |	/// This step searches for duplicate meshes and replaces them with references to the first mesh
				// Generate missing normals or UVs
				aiProcess_CalcTangentSpace          |	/// Calculates the tangents and bitangents for the imported meshes
				//aiProcess_GenSmoothNormals		|	/// Ignored if the mesh already has normals
				aiProcess_GenNormals                |
				aiProcess_GenUVCoords;				    /// Converts non-UV mappings (such as spherical or cylindrical mapping) to proper texture coordinate channels

			static const uint32_t Default_Mesh_Importer_Flags =
				/// Switch to engine conventions
				aiProcess_FlipUVs					|	/// DirectX style.
				aiProcess_MakeLeftHanded			|	/// DirectX style.
				// Validate and clean up
				aiProcess_ValidateDataStructure		|	/// Validates the imported scene data structure. This makes sure that all indices are valid, all animations and bones are linked correctly, all material references are correct
				aiProcess_FindDegenerates			|	/// Convert degenerate primitives to proper lines or points.
				aiProcess_FindInvalidData			|	/// This step searches all meshes for invalid data, such as zeroed normal vectors or invalid UV coords and removes / fixes them
				aiProcess_RemoveRedundantMaterials	|	/// Searches for redundant/unreferenced materials and removes them
				aiProcess_Triangulate				|	/// Triangulates all faces of all meshes
				aiProcess_JoinIdenticalVertices		|	/// Triangulates all faces of all meshes
				aiProcess_SortByPType				|	/// Splits meshes with more than one primitive type in homogeneous sub-meshes.
				aiProcess_FindInstances				|	/// This step searches for duplicate meshes and replaces them with references to the first mesh
				// Generate missing normals or UVs	
				aiProcess_CalcTangentSpace			|	/// Calculates the tangents and bitangents for the imported meshes
				//aiProcess_GenSmoothNormals		|	/// Ignored if the mesh already has normals
				aiProcess_GenNormals				|	
				aiProcess_GenUVCoords				|	/// Converts non-UV mappings (such as spherical or cylindrical mapping) to proper texture coordinate channels
				aiProcess_OptimizeGraph				|	/// Nodes without animations, bones, lights or cameras assigned are collapsed and joined.
				aiProcess_OptimizeMeshes;				/// Reduce the number of meshes.

			/// @brief Store all the current data used when importing a model/mesh.
			struct AssimpLoaderData
			{
				std::vector<Graphics::Vertex> Vertices;
				std::vector<u32> Indices;
				std::vector<std::string> Texture_File_Paths;
				std::vector<Texture2D*> Textures;

				std::string Directoy;

				Model* Model = nullptr;
				Mesh* Mesh = nullptr;
			};

			/// @brief Load a complete model.
			/// @param model 
			/// @param file_path 
			/// @param importer_flags
			/// @return bool
			static bool LoadModel(Model* model, std::string file_path, u32 importer_flags);
			/// @brief Load a single mesh. This will load only the first and top level mesh.
			/// @param mesh 
			/// @param file_path 
			/// @param importer_flags 
			/// @return bool
			static bool LoadMesh(Mesh* mesh, std::string file_path, u32 importer_flags);

			static bool ExportModel(Model* model, const std::string& file_path);

		private:
			/// @brief Process a single node from the aiScene.
			/// @param aiNode 
			/// @param aiScene 
			/// @param directory 
			/// @param loader_data 
			/// @param recursive 
			static void ProcessNode(aiNode* aiNode, const aiScene* aiScene, const std::string& directory, AssimpLoaderData& loader_data, bool recursive = true);
			
			/// @brief Process a single mesh. This sh
			/// @param mesh 
			/// @param aiScene 
			/// @param loader_data 
			static void ProcessMesh(aiMesh* mesh, const aiScene* aiScene, AssimpLoaderData& loader_data);
			
			/// @brief Extract materials from the aiScene.
			/// @param ai_material 
			/// @param ai_texture_type 
			/// @param texture_id 
			static void ExtractMaterialTextures(aiMaterial* ai_material, const AssimpLoaderData& known_data, AssimpLoaderData& mesh_data);
			
			static void ExtractMaterialType(aiMaterial* ai_material, aiTextureType ai_texture_type, const char* material_id, const AssimpLoaderData& known_data, AssimpLoaderData& mesh_data);

			/// @brief Load the textures which have been extracted.
			/// @param loader_data 
			static void LoadMaterialTextures(AssimpLoaderData& loader_data);

			/// @brief Optimize the vertices and indics data.
			/// @param loader_data 
			static void Optimize(AssimpLoaderData& loader_data);

			/// @brief Resize the GPU buffers for the model/mesh and upload all the data.
			/// @param loader_data 
			static void UploadGPUData(AssimpLoaderData& loader_data);
		};
	}
}
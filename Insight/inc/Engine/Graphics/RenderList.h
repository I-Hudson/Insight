#pragma once

#include "Engine/Core/Common.h"
#include "Engine/Graphics/Enums.h"

namespace Insight::Graphics
{
	class GPUBuffer;

	struct DrawCall
	{
		DrawCall() = default;

		struct
		{
			GPUBuffer* VertexBuffer;
			GPUBuffer* IndexBuffer;
		} Geometry;

		union
		{
			struct
			{
				u32 IndciesStart;
				u32 IndicesCount;
				u32 VertexStart;
				u32 VertexCount;
			};
		}Draw;

		glm::mat4 WorldTransform;
		glm::mat4 LocalTransform;

		//TODO: Remove this for a material has or pointer.
		std::string DiffuseTexture;
		std::vector<std::string> DiffuseTextureMeshBatch;
	};

	struct DrawCallList
	{
		std::vector<u32> DrawCalls;

		// TODO: Batch draw calls.

		void Clear()
		{
			DrawCalls.clear();
		}

		bool IsEmpty() const
		{
			return DrawCalls.empty();
		}
	};

	struct RenderListView
	{
		bool IsUsed;
		std::vector<DrawCall> DrawCalls;
		std::unordered_map<MaterialDrawMode, DrawCallList> DrawCallList;
		glm::mat4 Projection;
		glm::mat4 Transform;

		void Clear()
		{
			IsUsed = false;
			DrawCalls.clear();
			DrawCallList.clear();
		}

		void AddDrawCall(MaterialDrawMode drawMode, DrawCall drawCall);
	};

	struct RenderList
	{
		RenderListView MainCamera;
		std::vector<RenderListView> ExtraCameras;

		RenderListView DirectionalLight;

		void Clear()
		{
			MainCamera.Clear();
			ExtraCameras.clear();
			DirectionalLight.Clear();
		}

		static RenderList* GetFromPool();
		static void ReturnToPool(RenderList* renderList);
		static void ClearCache();
	};
}
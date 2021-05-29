#pragma once

#include "Engine/Core/Common.h"
#include "Engine/Graphics/Enums.h"

namespace Insight::Graphics
{
	class GPUBuffer;

	struct DrawCall
	{
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
			};
		}Draw;

		glm::mat4 WorldTransform;
		glm::mat4 LocalTransform;
		//TODO: Remove this for a material has or pointer.
		std::string TempTextureString;
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

	struct RenderList
	{
		std::vector<DrawCall> DrawCalls;
		std::unordered_map<MaterialDrawMode, DrawCallList> DrawCallList;
		glm::mat4 CameraTransform;
		glm::mat4 CameraProjection;

		void Clear()
		{
			DrawCalls.clear();
			DrawCallList.clear();
		}

		static RenderList* GetFromPool();
		static void ReturnToPool(RenderList* renderList);
		static void ClearCache();

		void AddDrawCall(MaterialDrawMode drawMode, DrawCall drawCall);
	};
}
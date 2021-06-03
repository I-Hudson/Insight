#include "ispch.h"
#include "Engine/Graphics/RenderList.h"

namespace Insight::Graphics
{
	std::vector<RenderList*> FreeRenderList;

	RenderList* RenderList::GetFromPool()
	{
		if (!FreeRenderList.empty())
		{
			RenderList* list = FreeRenderList.back();
			FreeRenderList.pop_back();
			return list;
		}
		return ::New<RenderList>();
	}

	void RenderList::ReturnToPool(RenderList* renderList)
	{
		if (!renderList)
		{
			return;
		}

		ASSERT(std::find(FreeRenderList.begin(), FreeRenderList.end(), renderList) == FreeRenderList.end() && "[RenderList::ReturnToPool] List is already in pool.");
		FreeRenderList.push_back(renderList);
		renderList->Clear();
	}

	void RenderList::ClearCache()
	{
		for (auto& list : FreeRenderList)
		{
			::Delete(list);
		}
		FreeRenderList.clear();
	}

	void RenderList::AddDrawCall(MaterialDrawMode drawNode, DrawCall drawCall)
	{
		u32 drawCallIndex = (u32)DrawCalls.size();
		DrawCalls.push_back(drawCall);
		DrawCallList[drawNode].DrawCalls.push_back(drawCallIndex);
	}
}
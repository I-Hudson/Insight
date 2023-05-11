#include "Graphics/GPUDeferedManager.h"

#include "Graphics/RHI/RHI_CommandList.h"

#include "Core/Profiler.h"
#include "Algorithm/Vector.h"

namespace Insight
{
	namespace Graphics
	{
		GPUDeferedRequest GPUDeferedManager::Push(GPUDeferedFunc func)
		{
			std::lock_guard lock(m_mutex);
			GPUDeferedRequest request;
			request.Id = m_requestId++;
			request.Func = std::move(func);
			m_requests.push_back(request);
			return request;
		}

		void GPUDeferedManager::Update(RHI_CommandList* cmd_list)
		{
			IS_PROFILE_FUNCTION();
			std::lock_guard lock(m_mutex);
			cmd_list->BeginTimeBlock("GPUDeferedManager::Update");
			for (auto const& request : m_requests)
			{
				request.Func(cmd_list);
			}
			cmd_list->EndTimeBlock();
			m_requests.clear();
		}

		void GPUDeferedManager::Remove(GPUDeferedRequest request)
		{
			IS_PROFILE_FUNCTION();
			std::lock_guard lock(m_mutex);
			while (true)
			{
				auto iter = Algorithm::VectorFindIf(m_requests, [request](const GPUDeferedRequest& item)
				{
					return request.Id == item.Id;
				});
				if (iter == m_requests.end())
				{
					break;
				}
				else
				{
					m_requests.erase(iter);
				}
			}
		}
	}
}
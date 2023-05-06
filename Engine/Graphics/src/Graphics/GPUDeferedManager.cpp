#include "Graphics/GPUDeferedManager.h"

#include "Graphics/RHI/RHI_CommandList.h"

#include "Core/Profiler.h"
#include "Algorithm/Vector.h"

namespace Insight
{
	namespace Graphics
	{
		void GPUDeferedManager::Push(void* pointer, GPUDeferedFunc func)
		{
			std::lock_guard lock(m_mutex);
			m_queue.push_back(std::make_pair(pointer, func));
		}

		void GPUDeferedManager::Update(RHI_CommandList* cmd_list)
		{
			IS_PROFILE_FUNCTION();
			std::lock_guard lock(m_mutex);
			cmd_list->BeginTimeBlock("GPUDeferedManager::Update");
			for (auto const& [pointer, func] : m_queue)
			{
				func(cmd_list);
			}
			cmd_list->EndTimeBlock();
			m_queue.clear();
		}
		void GPUDeferedManager::Remove(void* pointer)
		{
			IS_PROFILE_FUNCTION();
			std::lock_guard lock(m_mutex);
			while (true)
			{
				auto iter = Algorithm::VectorFindIf(m_queue, [pointer](const std::pair<void*, GPUDeferedFunc>& pair)
				{
					return pointer == pair.first;
				});
				if (iter == m_queue.end())
				{
					break;
				}
				else
				{
					m_queue.erase(iter);
				}
			}
		}
	}
}
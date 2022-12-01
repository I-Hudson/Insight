#include "Graphics/GPUDeferedManager.h"

#include "Core/Profiler.h"

namespace Insight
{
	namespace Graphics
	{
		void GPUDeferedManager::Push(GPUDeferedFunc func)
		{
			std::lock_guard lock(m_mutex);
			m_queue.push_back(func);
		}

		void GPUDeferedManager::Update(RHI_CommandList* cmd_list)
		{
			IS_PROFILE_FUNCTION();
			std::lock_guard lock(m_mutex);
			for (GPUDeferedFunc& func : m_queue)
			{
				func(cmd_list);
			}
			m_queue.clear();
		}
	}
}
#include "Graphics/GPUDeferedManager.h"

namespace Insight
{
	namespace Graphics
	{
		void GPUDeferedManager::Push(GPUDeferedFunc func)
		{
			m_queue.push_back(func);
		}

		void GPUDeferedManager::Update(RHI_CommandList* cmd_list)
		{
			for (GPUDeferedFunc& func : m_queue)
			{
				func(cmd_list);
			}
			m_queue.clear();
		}
	}
}
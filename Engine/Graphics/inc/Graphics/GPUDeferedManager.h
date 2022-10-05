#pragma once

#include "Core/Singleton.h"

#include <vector>
#include <functional>

namespace Insight
{
	namespace Graphics
	{
		class RHI_CommandList;
		/// @brief Manage anything which is defered for GPU items.
		class GPUDeferedManager : public Core::Singleton<GPUDeferedManager>
		{
			using GPUDeferedFunc = std::function<void(RHI_CommandList*)>;
		public:
			void Push(GPUDeferedFunc func);
			void Update(RHI_CommandList* cmd_list);
		private:
			std::vector<GPUDeferedFunc> m_queue;
		};
	}
}
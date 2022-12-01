#pragma once

#include "Graphics/Defines.h"

#include "Core/Singleton.h"

#include <vector>
#include <functional>
#include <mutex>

namespace Insight
{
	namespace Graphics
	{
		class RHI_CommandList;
		/// @brief Manage anything which is defered for GPU items.
		class IS_GRAPHICS GPUDeferedManager : public Core::Singleton<GPUDeferedManager>
		{
			using GPUDeferedFunc = std::function<void(RHI_CommandList*)>;
		public:
			void Push(GPUDeferedFunc func);
			void Update(RHI_CommandList* cmd_list);
		private:
			std::vector<GPUDeferedFunc> m_queue;
			std::mutex m_mutex;
		};
	}
}
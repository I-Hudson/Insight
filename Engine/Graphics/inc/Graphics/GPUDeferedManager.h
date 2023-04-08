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
			THREAD_SAFE;
			using GPUDeferedFunc = std::function<void(RHI_CommandList*)>;
		public:
			void Push(void* pointer, GPUDeferedFunc func);
			void Update(RHI_CommandList* cmd_list);
			void Remove(void* pointer);
		private:
			std::vector<std::pair<void*, GPUDeferedFunc>> m_queue;
			std::mutex m_mutex;
		};
	}
}
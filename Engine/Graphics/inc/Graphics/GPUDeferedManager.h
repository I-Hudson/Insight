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
		using GPUDeferedFunc = std::function<void(RHI_CommandList*)>;

		struct GPUDeferedRequest
		{
			u64 Id = 0;
			bool Cancelled = false;
			GPUDeferedFunc Func;
		};

		/// @brief Manage anything which is defered for GPU items.
		class IS_GRAPHICS GPUDeferedManager : public Core::Singleton<GPUDeferedManager>
		{
			THREAD_SAFE;
		public:
			GPUDeferedRequest Push(GPUDeferedFunc func);
			void Update(RHI_CommandList* cmd_list);
			void Remove(GPUDeferedRequest request);

		private:
			std::vector<GPUDeferedRequest> m_requests;
			u64 m_requestId = 0;
			std::mutex m_mutex;
		};
	}
}
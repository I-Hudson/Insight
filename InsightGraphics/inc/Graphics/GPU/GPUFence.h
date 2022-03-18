#pragma once

#include <list>

namespace Insight
{
	namespace Graphics
	{
		class GPUFenceManager;

		class GPUFence
		{
		public:
			virtual ~GPUFence() { };

			virtual void Wait() = 0;
			virtual void Reset() = 0;
			virtual bool IsSignaled() = 0;

		protected:
			virtual void Create() = 0;
			virtual void Destroy() = 0;

			friend class GPUFenceManager;
		};

		class GPUFenceManager
		{
		public:
			GPUFenceManager();
			~GPUFenceManager();

			static GPUFenceManager& Instance()
			{
				static GPUFenceManager ins;
				return ins;
			}

			GPUFence* GetFence();
			void ReturnFence(GPUFence* fence);

			void Destroy();

		private:
			std::list<GPUFence*> m_inUseFences;
			std::list<GPUFence*> m_freeFences;
		};
	}
}
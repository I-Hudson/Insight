#pragma once

#include <list>

namespace Insight
{
	namespace Graphics
	{
		class GPUSemaphoreManager;

		class GPUSemaphore
		{
		public:
			virtual ~GPUSemaphore() { }

			virtual void Signal() = 0;

		private:
			static GPUSemaphore* New();

		protected:
			virtual void Create(bool signaled) = 0;
			virtual void Destroy() = 0;

			friend class GPUSemaphoreManager;
		};

		class GPUSemaphoreManager
		{
		public:
			~GPUSemaphoreManager();

			static GPUSemaphoreManager& Instance()
			{
				static GPUSemaphoreManager ins;
				return ins;
			}

			GPUSemaphore* GetOrCreateSemaphore(bool signaled = false);
			void ReturnSemaphore(GPUSemaphore*& semaphore);

			void Destroy();

		protected:
			std::list<GPUSemaphore*> m_inUseSemaphroes;
			std::list<GPUSemaphore*> m_freeSemaphroes;
		};
	}
}
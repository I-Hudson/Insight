#pragma once

#include "Engine/Core/Core.h"
#include "Engine/Threading/ThreadPool.hpp"

	namespace Threading
	{
		template<U32 ThreadCount, typename TThreadFuncReturnValue, typename TThreadData, typename... TThreadInputData>
		class ThreadCollection
		{
			using ThreadFunc = std::function< TThreadFuncReturnValue(TThreadData&)>;
			using DefaultAssignThreadFunc = std::function<void(std::array<TThreadData, ThreadCount>&, TThreadInputData...)>;
			using ReturnCollectionVector = std::vector<std::future<TThreadFuncReturnValue>>;

		public:
			explicit ThreadCollection(ThreadFunc&& threadFunc, DefaultAssignThreadFunc&& assignThreadFunc)
			{
				m_threadFunc = threadFunc;
				m_assignFunc = assignThreadFunc;
			}

			void AddData(const TThreadInputData&... args)
			{
				m_assignFunc(m_threadsData, args...);
			}

			[[nodiscard]] ReturnCollectionVector Execute()
			{
				ReturnCollectionVector vec;
				vec.reserve(ThreadCount);

				for (auto& data : m_threadsData)
				{
					vec.push_back(m_threadPool.push(m_threadFunc, data));
				}

				return vec;
			}

			void Wait()
			{
				m_threadPool.join();
			}

		private:
			thread_pool m_threadPool{ ThreadCount };
			std::array<TThreadData, ThreadCount> m_threadsData;
			ThreadFunc m_threadFunc;
			DefaultAssignThreadFunc  m_assignFunc;
			bool m_waitHasBeenCalled;
		};
	}
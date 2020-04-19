#pragma once

#include "Insight/Core.h"
#include "Insight/InsightAlias.h"
#include "Insight/Memory/MemoryUtils.h"

namespace Insight
{
	namespace Memory
	{
		class IS_API StackAllocator
		{
		public:
			typedef Size Marker;

			StackAllocator() = delete;
			explicit StackAllocator(Size stackSize);
			~StackAllocator();

			void* Alloc(Size size, Byte alignment = MemoryUtlis::Alignment);

			template<typename T, typename... Args>
			T* New(Args...);

			template<typename T, typename... Args>
			T* NewArr(Size length, Byte align = MemoryUtlis::Alignment);

			void PrintUsed();
			Size GetUsedAmount() const { return m_top; }

			void FreeToMarker(const Marker marker);

			void FreeAll() { m_top = 0; }

			const Marker GetMarker() const { return m_top; }

		private:
			Marker m_top;
			Size m_totalSize;
			void* m_bottom;
			PtrInt m_bottomAddress;
		};

		template<typename T, typename... Args>
		inline T* StackAllocator::New(Args... argList)
		{
			void* mem = Alloc(sizeof(T));
			return new (mem) T(argList...);
		}

		template<typename T, typename ...Args>
		inline T* StackAllocator::NewArr(Size length, Byte align)
		{
			void* alloc = Alloc(sizeof(T) * length, align);
			char* allocAddress = static_cast<char*>(alloc);
			for (int i = 0; i < length; ++i) new (allocAddress + i * sizeof(T)) T;
			return static_cast<T*>(alloc);
		}

	}
}

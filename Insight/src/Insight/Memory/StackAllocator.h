#pragma once

#include "Insight/Core/Core.h"
#include "Insight/Memory/MemoryUtils.h"

	namespace Memory
	{
		class IS_API StackAllocator
		{
		public:
			typedef Size Marker;

			StackAllocator() = delete;
			StackAllocator(Size stackSize);
			~StackAllocator();

			void* Alloc(Size size, Byte alignment = MemoryUtlis::Alignment);

			template<typename T, typename... Args>
			T* New(Args...);

			template<typename T>
			T* NewArr(Size length, Byte align = MemoryUtlis::Alignment);

			void PrintUsed();
			Size GetUsedAmount() const { return m_top; }

			void FreeToMarker(const Marker marker);

			void FreeAll() { m_top = 0; }

			Marker GetMarker() { return m_top; }

		private:
			Marker m_top;
			Size m_totalSize;
			void* m_startPtr;
			PtrInt m_startAddress;
			bool m_subAllocation;

			struct AllocHeader
			{
				char Padding;
			};
		};

		template<typename T, typename... Args>
		inline T* StackAllocator::New(Args... argList)
		{
			void* mem = Alloc(sizeof(T));
			return new (mem) T(argList...);
		}

		template<typename T>
		inline T* StackAllocator::NewArr(Size length, Byte align)
		{
			void* alloc = Alloc(sizeof(T) * length, align);
			return static_cast<T*>(alloc);
		}

	}
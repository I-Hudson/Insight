#pragma once

#include "Core/TypeAlias.h"
#include "Enums.h"
#include <map>

namespace Insight
{
	namespace Graphics
	{
		class GPUBufferManager;

		struct GPUBufferCreateInfo
		{
			GPUBufferCreateInfo()
			{ }
			GPUBufferCreateInfo(int size, GPUBufferType type)
				: Size(size), Type(type)
			{ }

			int Size = 0;
			GPUBufferType Type = GPUBufferType::Invalid;
		};

		class GPUBuffer
		{
		public:
			virtual ~GPUBuffer() { }

			int GetSize() const { return m_info.Size; }
			GPUBufferType GetType() const { return m_info.Type; }

			bool SetData(const void* data, u64 size) { return Upload(data, size, 0); }
			bool SetData(const void* data, u64 size, u64 offset) { return Upload(data, size, offset); };

		private:
			static GPUBuffer* New();

		protected:
			virtual void Create(GPUBufferCreateInfo desc) = 0;
			virtual void Destroy() = 0;
			virtual bool Upload(const void* data, u64 size, u64 offset) = 0;

		protected:
			GPUBufferCreateInfo m_info;

			friend class GPUBufferManager;
		};

		class GPUBufferManager
		{
		public:
			GPUBufferManager();
			~GPUBufferManager();

			static GPUBufferManager& Instance()
			{
				static GPUBufferManager ins;
				return ins;
			}

			GPUBuffer* CreateBuffer(std::string key, GPUBufferCreateInfo createInfo);
			GPUBuffer* GetBuffer(std::string key);
			void DestroyBuffer(std::string key);
			void DestroyBuffer(GPUBuffer* buffer);

			void Destroy();

		private:
			std::map<std::string, GPUBuffer*> m_buffers;
		};
	}
}
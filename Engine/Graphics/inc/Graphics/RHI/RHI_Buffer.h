#pragma once

#include "Graphics/RHI/RHI_Resource.h"
#include "Graphics/RHI/RHI_UploadQueue.h"

namespace Insight
{
	namespace Graphics
	{
		class RenderContext;
		class RHI_Buffer;

		//// <summary>
		//// Override default behavior for RHI_Buffer when created.
		//// </summary>
		struct IS_GRAPHICS RHI_Buffer_Overrides
		{
			bool Force_Host_Writeable = false;
			bool AllowUnorderedAccess = false;

			DeviceUploadStatus InitialUploadState = DeviceUploadStatus::NotUploaded;
		};

		class IS_GRAPHICS RHI_BufferView
		{
		public:
			RHI_BufferView();
			RHI_BufferView(RHI_Buffer* buffer);
			RHI_BufferView(RHI_Buffer* buffer, u64 offset, u64 size);
			//RHI_BufferView(const RHI_BufferView& other);
			//RHI_BufferView(RHI_BufferView&& other);

			//RHI_BufferView& operator=(const RHI_BufferView& other);
			//RHI_BufferView& operator=(RHI_BufferView&& other);

			bool operator==(const RHI_BufferView& other) const;
			bool operator!=(const RHI_BufferView& other) const;

			bool IsValid() const { return m_buffer; }

			constexpr RHI_Buffer* GetBuffer() const { return m_buffer; }
			constexpr u64 GetOffset() const { return m_offset; }
			constexpr u64 GetSize() const { return m_size; }

			u64 UAVStartIndex = 0;
			u64 UAVNumOfElements = 0;
			u64 Stride = 0;

		private:
			RHI_Buffer* m_buffer = nullptr;
			u64 m_offset = 0;
			u64 m_size = 0;

			friend std::hash<RHI_BufferView>;
		};

		struct RHI_BufferCreateInfo
		{
			BufferType BufferType;
			u64 SizeBytes;
			u64 Stride;
			RHI_Buffer_Overrides Overrides;
		};

		class IS_GRAPHICS RHI_Buffer : public RHI_Resource
		{
		public:
			static RHI_Buffer* New();

			Byte* GetMappedData() const;

			virtual ~RHI_Buffer() { }

			virtual void Create(RenderContext* context, const RHI_BufferCreateInfo& createInfo) { Create(context, createInfo.BufferType, createInfo.SizeBytes, createInfo.Stride, createInfo.Overrides); }
			virtual void Create(RenderContext* context, BufferType bufferType, u64 sizeBytes, u64 stride, RHI_Buffer_Overrides overrides) = 0;
			virtual RHI_BufferView Upload(const void* data, u64 sizeInBytes, u64 offset, u64 alignment) = 0;
			virtual RHI_BufferView Upload(RHI_Buffer* srcBuffer) = 0;

			RHI_BufferView Upload(const void* data, u64 sizeInBytes) { return Upload(data, sizeInBytes, 0, 0); }
			RPtr<RHI_UploadQueueRequest> QueueUpload(void* data, int sizeInBytes);

			virtual std::vector<Byte> Download() = 0;
			virtual void Resize(u64 newSizeBytes) = 0;


			RHI_BufferView GetView(u64 offset, u64 size);
			/// <summary>
			/// Returns the size of the buffer in bytes.
			/// </summary>
			/// <returns></returns>
			u64 GetSize() const { return m_size; }
			u64 GetStride() const { return m_stride; }
			BufferType GetType() const { return m_bufferType; }

		private:
			void OnUploadComplete(RHI_UploadQueueRequest* request);

		protected:
			BufferType m_bufferType;
			u64 m_size = 0;
			u64 m_stride = 0;
			RHI_Buffer_Overrides m_overrides = { };
			Byte* m_mappedData = nullptr;


			friend class RenderContext;
			friend class RHI_DynamicBuffer;
		};

		class RHI_DynamicBuffer
		{
		public:
			void Create(RenderContext* context);
			void Release();

			RHI_BufferView Upload(const void* data, u64 sizeInBytes);

			RHI_Buffer* GetBuffer() const { return m_buffer; }
			RHI_BufferView GetView(u64 offset, u64 size) const;
			void Resize(u64 newSizeInBytes);
			void Reset();

		private:
			RenderContext* m_context = nullptr;
			RHI_Buffer* m_buffer = nullptr;
			u64 m_size = 0;
			u64 m_capacity = 0;
		};
	}
}

namespace std
{
	using RHI_BufferView = Insight::Graphics::RHI_BufferView;
	
	template<>
	struct hash<RHI_BufferView>
	{
		_NODISCARD size_t operator()(const RHI_BufferView& val) const
		{
			u64 hash = 0;
			HashCombine(hash, val.m_buffer);
			HashCombine(hash, val.m_offset);
			HashCombine(hash, val.m_size);
			return hash;
		}
	};
}
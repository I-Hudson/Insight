#pragma once

#include "Core/Memory.h"
#include "Core/TypeAlias.h"

#include "Graphics/Defines.h"
#include "Graphics/Enums.h"

#ifdef IS_RESOURCE_HANDLES_ENABLED
#include "Graphics/RHI/RHI_Handle.h"
#endif

#include <vector>
#include <functional>
#include <mutex>

namespace Insight
{
	namespace Graphics
	{
		class RHI_CommandList;
		class RHI_Buffer;
		class RHI_Resource;
		class RHI_Texture;
		struct RHI_UploadQueueRequestInternal;
		class RHI_UploadQueue;

		using RHI_UploadQueueFunction = std::function<void(const RHI_UploadQueueRequestInternal*, RHI_CommandList*)>;

		enum class RHI_UploadTypes
		{
			Buffer,
			Texture
		};

		/// <summary>
		/// Struct returned with the current status of the uploaded resource.
		/// </summary>
		struct IS_GRAPHICS RHI_UploadQueueRequest
		{
			std::atomic<DeviceUploadStatus> Status = DeviceUploadStatus::NotUploaded;
			RHI_Resource* Resource = nullptr;
			Core::Delegate<RHI_UploadQueueRequest*> OnUploadCompleted;
		};

		struct RHI_UploadQueueRequestInternal
		{
			RHI_UploadQueueRequestInternal(RHI_UploadQueueFunction function, RHI_Resource* resource, u64 sizeInBytes);
			
			RHI_UploadQueueFunction UploadFunction;
			u64 SizeInBytes;
			RPtr<RHI_UploadQueueRequest> Request;
			RHI_CommandList* CommandList;
			bool Cancelled = false;

		private:
			void OnWorkComplete();

			friend class RHI_UploadQueue;
		};

		/// <summary>
		/// Helper class for uploading any resource from host (RAM) to device (GPU).
		/// </summary>
		class IS_GRAPHICS RHI_UploadQueue
		{
			THREAD_SAFE;
		public:

			RHI_UploadQueue();
			RHI_UploadQueue(const RHI_UploadQueue& other) = delete;
			RHI_UploadQueue(RHI_UploadQueue&& other) = delete;
			~RHI_UploadQueue();

			void Init();
			void Destroy();

			RPtr<RHI_UploadQueueRequest> UploadBuffer(const void* data, u64 sizeInBytes, RHI_Buffer* buffer);
			/// <summary>
			/// Add a new upload request for a texture to the queue.
			/// </summary>
			/// <param name="data"></param>
			/// <param name="sizeInBytes"></param>
			RPtr<RHI_UploadQueueRequest> UploadTexture(const void* data, u64 sizeInBytes, RHI_Texture* texture);
#ifdef IS_RESOURCE_HANDLES_ENABLED
			void UploadTexture(const void* data, u64 sizeInBytes, RHI_Handle<Texture> textureHandle);
#endif
			/// <summary>
			/// Upload the data to the device.
			/// </summary>
			/// <param name="cmdList"></param>
			void UploadToDevice(RHI_CommandList* cmdList);

			void RemoveRequest(RHI_UploadQueueRequest* request);

		private:
			void UploadDataToStagingBuffer(const void* data, u64 sizeInBytes, RHI_UploadTypes uploadType, RPtr<RHI_UploadQueueRequestInternal>& uploadRequest);

		private:
			/// <summary>
			/// All queued uploads to be completed.
			/// </summary>
			std::vector<RPtr<RHI_UploadQueueRequestInternal>> m_queuedUploads;
			std::vector<RPtr<RHI_UploadQueueRequestInternal>> m_runningUploads;
			/// <summary>
			/// Buffer to store all data to be uploaded. (This is used for staging resources).
			/// </summary>
			RHI_Buffer* m_uploadStagingBuffer = nullptr;
			u64 m_stagingBufferOffset = 0;

			/// <summary>
			/// The current offset this frame when uploading all the resources.
			/// </summary>
			u64 m_frameUploadOffset = 0;

			const u64 c_UploadBufferMaxSize = 4_MB;

			std::mutex m_mutex;
		};
	}
}
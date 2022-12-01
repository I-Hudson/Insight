#pragma once

#include "Core/Memory.h"
#include "Core/TypeAlias.h"

#include "Graphics/Defines.h"
#include "Graphics/Enums.h"

#include <vector>
#include <functional>

namespace Insight
{
	namespace Graphics
	{
		class RHI_CommandList;
		class RHI_Buffer;
		class RHI_Texture;
		struct RHI_UploadQueueRequestInternal;
		class RHI_UploadQueue;

		using RHI_UploadQueueFunction = std::function<void(const RHI_UploadQueueRequestInternal*, RHI_CommandList*)>;

		/// <summary>
		/// Struct returned with the current status of the uploaded resource.
		/// </summary>
		struct IS_GRAPHICS RHI_UploadQueueRequest
		{
			std::atomic<DeviceUploadStatus> Status = DeviceUploadStatus::Unknown;
			Core::Delegate<RHI_UploadQueueRequest*> OnUploadCompleted;
		};

		struct RHI_UploadQueueRequestInternal
		{
			RHI_UploadQueueRequestInternal(RHI_UploadQueueFunction function, u64 sizeInBytes);
			RHI_UploadQueueFunction UploadFunction;
			u64 SizeInBytes;
			RPtr<RHI_UploadQueueRequest> Request;
			RHI_CommandList* CommandList;

		private:
			void OnWorkComplete();

			friend class RHI_UploadQueue;
		};

		/// <summary>
		/// Helper class for uploading any resource from host (RAM) to device (GPU).
		/// </summary>
		class IS_GRAPHICS RHI_UploadQueue
		{
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
			/// <summary>
			/// Upload the data to the device.
			/// </summary>
			/// <param name="cmdList"></param>
			void UploadToDevice(RHI_CommandList* cmdList);

		private:
			void UploadDataToStagingBuffer(const void* data, u64 sizeInBytes);

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

			std::mutex m_mutex;
		};
	}
}
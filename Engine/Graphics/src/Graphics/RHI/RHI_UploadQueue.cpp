#include "Graphics/RHI/RHI_UploadQueue.h"
#include "Graphics/RenderContext.h"

namespace Insight
{
	namespace Graphics
	{
		RHI_UploadQueueRequestInternal::RHI_UploadQueueRequestInternal(RHI_UploadQueueFunction function, u64 sizeInBytes)
			: UploadFunction(std::move(function))
			, SizeInBytes(std::move(sizeInBytes))
		{
			Request = MakeRPtr<RHI_UploadQueueRequest>();
		}

		RHI_UploadQueue::RHI_UploadQueue()
		{
			m_uploadStagingBuffer = Renderer::CreateRawBuffer(1_MB, 
				RHI_Buffer_Overrides
				{ 
					/*Force_Host_Writeable=*/true
				});
		}
		RHI_UploadQueue::~RHI_UploadQueue()
		{
			m_queuedUploads.clear();
			Renderer::FreeRawBuffer(m_uploadStagingBuffer);
		}

		RPtr<RHI_UploadQueueRequest> RHI_UploadQueue::UploadTexture(const void* data, u64 sizeInBytes, RHI_Texture* texture)
		{
			ASSERT(Platform::IsMainThread());

			UploadDataToStagingBuffer(data, sizeInBytes);

			m_queuedUploads.push_back(
				RHI_UploadQueueRequestInternal(
					[&](const RHI_UploadQueueRequestInternal& request, RHI_CommandList* cmdList)
					{
						request.Request->Status = DeviceUploadStatus::Uploading;
						cmdList->CopyBufferToImage(texture, m_uploadStagingBuffer, m_frameUploadOffset);
						m_frameUploadOffset += request.SizeInBytes;
				}, sizeInBytes));

			return m_queuedUploads.back().Request;
		}

		void RHI_UploadQueue::UploadToDevice(RHI_CommandList* cmdList)
		{
			m_frameUploadOffset = 0;
			for (size_t i = 0; i < m_queuedUploads.size(); ++i)
			{
				m_queuedUploads.at(i).UploadFunction(m_queuedUploads.at(i), cmdList);
			}
			m_queuedUploads.clear();
			m_stagingBufferOffset = 0;
		}

		void RHI_UploadQueue::UploadDataToStagingBuffer(const void* data, u64 sizeInBytes)
		{
			// Check we have enough space.
			if (m_stagingBufferOffset + sizeInBytes > m_uploadStagingBuffer->GetSize())
			{
				m_uploadStagingBuffer->Resize(m_uploadStagingBuffer->GetSize() * 1.8f);
			}
			// Upload the data.
			m_uploadStagingBuffer->Upload(data, sizeInBytes, m_stagingBufferOffset);
			m_stagingBufferOffset += sizeInBytes;
		}
	}
}
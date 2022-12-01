#include "Graphics/RHI/RHI_UploadQueue.h"
#include "Graphics/RenderContext.h"

#include <iterator>

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

		void RHI_UploadQueueRequestInternal::OnWorkComplete()
		{
			CommandList->OnWorkCompleted.Unbind<&RHI_UploadQueueRequestInternal::OnWorkComplete>(this);
			Request->Status = DeviceUploadStatus::Completed;
			Request->OnUploadCompleted(Request.Get());
		}

		RHI_UploadQueue::RHI_UploadQueue()
		{
		}

		RHI_UploadQueue::~RHI_UploadQueue()
		{
			Destroy();
		}

		void RHI_UploadQueue::Init()
		{
			std::lock_guard lock(m_mutex);
			if (!m_uploadStagingBuffer)
			{
				m_uploadStagingBuffer = Renderer::CreateRawBuffer(1_MB,
					RHI_Buffer_Overrides
					{
						/*Force_Host_Writeable=*/true
					});
			}
		}

		void RHI_UploadQueue::Destroy()
		{
			std::lock_guard lock(m_mutex);
			m_queuedUploads.clear();
			Renderer::FreeRawBuffer(m_uploadStagingBuffer);
			m_uploadStagingBuffer = nullptr;
		}

		RPtr<RHI_UploadQueueRequest> RHI_UploadQueue::UploadBuffer(const void* data, u64 sizeInBytes, RHI_Buffer* buffer)
		{
			IS_PROFILE_FUNCTION();
			ASSERT(Platform::IsMainThread());

			UploadDataToStagingBuffer(data, sizeInBytes);

			std::lock_guard lock(m_mutex);
			return RPtr<RHI_UploadQueueRequest>();
		}

		RPtr<RHI_UploadQueueRequest> RHI_UploadQueue::UploadTexture(const void* data, u64 sizeInBytes, RHI_Texture* texture)
		{
			IS_PROFILE_FUNCTION();
			ASSERT(Platform::IsMainThread());

			UploadDataToStagingBuffer(data, sizeInBytes);

			std::lock_guard lock(m_mutex);
			m_queuedUploads.push_back(MakeRPtr<
				RHI_UploadQueueRequestInternal>(
					[=](const RHI_UploadQueueRequestInternal* request, RHI_CommandList* cmdList)
					{
						request->Request->Status = DeviceUploadStatus::Uploading;
						cmdList->CopyBufferToImage(texture, m_uploadStagingBuffer, m_frameUploadOffset);
						m_frameUploadOffset += request->SizeInBytes;
				}, sizeInBytes));

			return m_queuedUploads.back()->Request;
		}

		void RHI_UploadQueue::UploadToDevice(RHI_CommandList* cmdList)
		{
			IS_PROFILE_FUNCTION();
			ASSERT(Platform::IsMainThread());

			m_frameUploadOffset = 0;

			// Remove all completed requests from m_runningUploads.

			std::vector<RHI_UploadQueueRequestInternal*> completedRequests;
			for (auto const& uploadRequest : m_runningUploads)
			{
				if (uploadRequest->Request->Status == DeviceUploadStatus::Completed)
				{
					completedRequests.push_back(uploadRequest.Get());
				}
			}

			for (auto const& requestToRemove : completedRequests)
			{
				auto itr = std::find_if(m_runningUploads.begin(), m_runningUploads.end(), [&requestToRemove](const RPtr<RHI_UploadQueueRequestInternal>& request)
					{
						return requestToRemove == request.Get();
					});
				if (itr != m_runningUploads.end())
				{
					m_runningUploads.erase(itr);
				}
			}

			// Bind our work completed function.
			for (size_t i = 0; i < m_queuedUploads.size(); ++i)
			{
				m_queuedUploads.at(i)->CommandList = cmdList;
				m_queuedUploads.at(i)->CommandList->OnWorkCompleted.Bind<&RHI_UploadQueueRequestInternal::OnWorkComplete>(m_queuedUploads.at(i).Get());
				// Call the upload functions.
				m_queuedUploads.at(i)->UploadFunction(m_queuedUploads.at(i).Get(), cmdList);
			}
			// Move all  our requests to the running vector.
			std::move(m_queuedUploads.begin(), m_queuedUploads.end(), std::back_inserter(m_runningUploads));
			m_queuedUploads.clear();

			m_stagingBufferOffset = 0;
		}

		void RHI_UploadQueue::UploadDataToStagingBuffer(const void* data, u64 sizeInBytes)
		{
			IS_PROFILE_FUNCTION();
			ASSERT(Platform::IsMainThread());

			std::lock_guard lock(m_mutex);
			// Check we have enough space.
			if (m_stagingBufferOffset + sizeInBytes > m_uploadStagingBuffer->GetSize())
			{
				u64 newSizeBytes = m_uploadStagingBuffer->GetSize();
				while (newSizeBytes < m_stagingBufferOffset + sizeInBytes)
				{
					newSizeBytes *= 1.8f;
				}
				m_uploadStagingBuffer->Resize(newSizeBytes);
			}
			// Upload the data.
			m_uploadStagingBuffer->Upload(data, sizeInBytes, m_stagingBufferOffset);
			m_stagingBufferOffset += sizeInBytes;
		}
	}
}
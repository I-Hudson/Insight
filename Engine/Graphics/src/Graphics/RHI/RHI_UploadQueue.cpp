#include "Graphics/RHI/RHI_UploadQueue.h"
#include "Graphics/RenderContext.h"
#include "Graphics/RHI/RHI_Resource.h"

#include "Core/Profiler.h"
#include "Algorithm/Vector.h"

#include <iterator>

namespace Insight
{
	namespace Graphics
	{
		RHI_UploadQueueRequestInternal::RHI_UploadQueueRequestInternal(RHI_UploadQueueFunction function, RHI_Resource* resource, u64 sizeInBytes)
			: UploadFunction(std::move(function))
			, SizeInBytes(std::move(sizeInBytes))
		{
			Request = MakeRPtr<RHI_UploadQueueRequest>();
			Request->Resource = resource;
		}

		void RHI_UploadQueueRequestInternal::OnWorkComplete()
		{
			CommandList->OnWorkCompleted.Unbind<&RHI_UploadQueueRequestInternal::OnWorkComplete>(this);
			Request->Status = DeviceUploadStatus::Completed;
			Request->Resource->m_uploadStatus = DeviceUploadStatus::Completed;
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
			//ASSERT(RenderContext::Instance().IsRenderThread());

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
			//ASSERT(RenderContext::Instance().());

			std::lock_guard lock(m_mutex);
			m_queuedUploads.clear();
			Renderer::FreeRawBuffer(m_uploadStagingBuffer);
			m_uploadStagingBuffer = nullptr;
		}

		RPtr<RHI_UploadQueueRequest> RHI_UploadQueue::UploadBuffer(const void* data, u64 sizeInBytes, RHI_Buffer* buffer)
		{
			IS_PROFILE_FUNCTION();

			UploadDataToStagingBuffer(data, sizeInBytes, RHI_UploadTypes::Buffer);

			std::lock_guard lock(m_mutex);
			m_queuedUploads.push_back(MakeRPtr<
				RHI_UploadQueueRequestInternal>(
					[=](const RHI_UploadQueueRequestInternal* request, RHI_CommandList* cmdList)
					{
						if (request->Cancelled)
						{
							return;
						}

						request->Request->Status = DeviceUploadStatus::Uploading;
						cmdList->CopyBufferToBuffer(buffer, 0, m_uploadStagingBuffer, m_frameUploadOffset, request->SizeInBytes);
						m_frameUploadOffset += request->SizeInBytes;
					}, buffer, sizeInBytes));

			return m_queuedUploads.back()->Request;
		}

		RPtr<RHI_UploadQueueRequest> RHI_UploadQueue::UploadTexture(const void* data, u64 sizeInBytes, RHI_Texture* texture)
		{
			IS_PROFILE_FUNCTION();

			UploadDataToStagingBuffer(data, sizeInBytes, RHI_UploadTypes::Texture);

			std::lock_guard lock(m_mutex);
			m_queuedUploads.push_back(MakeRPtr<
				RHI_UploadQueueRequestInternal>(
					[=](const RHI_UploadQueueRequestInternal* request, RHI_CommandList* cmdList)
					{
						if (request->Cancelled)
						{
							return;
						}

						PipelineBarrier barreir;
						barreir.SrcStage = +PipelineStageFlagBits::TopOfPipe;
						barreir.DstStage = +PipelineStageFlagBits::Transfer;

						ImageBarrier imageBarrier;
						imageBarrier.SrcAccessFlags = AccessFlagBits::None;
						imageBarrier.DstAccessFlags = AccessFlagBits::TransferWrite;
						imageBarrier.OldLayout = texture->GetLayout();
						imageBarrier.NewLayout = ImageLayout::TransforDst;
						imageBarrier.SubresourceRange = ImageSubresourceRange::SingleMipAndLayer(ImageAspectFlagBits::Colour);
						imageBarrier.Image = texture;
						barreir.ImageBarriers.push_back(imageBarrier);

						cmdList->PipelineBarrier(barreir);
						barreir = { };

						request->Request->Status = DeviceUploadStatus::Uploading;
						cmdList->CopyBufferToImage(texture, m_uploadStagingBuffer, m_frameUploadOffset);
						m_frameUploadOffset += request->SizeInBytes;

						barreir.SrcStage = +PipelineStageFlagBits::Transfer;
						barreir.DstStage = +PipelineStageFlagBits::FragmentShader;

						imageBarrier.SrcAccessFlags = AccessFlagBits::TransferWrite;
						imageBarrier.DstAccessFlags = AccessFlagBits::ShaderRead;
						imageBarrier.OldLayout = texture->GetLayout();
						imageBarrier.NewLayout = ImageLayout::ShaderReadOnly;
						imageBarrier.SubresourceRange = ImageSubresourceRange::SingleMipAndLayer(ImageAspectFlagBits::Colour);
						imageBarrier.Image = texture;

						barreir.ImageBarriers.push_back(imageBarrier);
						cmdList->PipelineBarrier(barreir);

				}, texture, sizeInBytes));

			return m_queuedUploads.back()->Request;
		}

		void RHI_UploadQueue::UploadToDevice(RHI_CommandList* cmdList)
		{
			IS_PROFILE_FUNCTION();
			ASSERT(RenderContext::Instance().IsRenderThread());

			std::lock_guard lock(m_mutex);
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

		void RHI_UploadQueue::RemoveRequest(RHI_UploadQueueRequest* request)
		{
			IS_PROFILE_FUNCTION();
			std::lock_guard lock(m_mutex);

			for (const RPtr<RHI_UploadQueueRequestInternal>& internalRequest : m_queuedUploads)
			{
				if (request == internalRequest->Request.Get())
				{
					internalRequest->Cancelled = true;
					break;
				}
			}
		}

		void RHI_UploadQueue::UploadDataToStagingBuffer(const void* data, u64 sizeInBytes, RHI_UploadTypes uploadType)
		{
			IS_PROFILE_FUNCTION();

			std::lock_guard lock(m_mutex);
			// Check we have enough space.
			if (m_stagingBufferOffset + sizeInBytes > m_uploadStagingBuffer->GetSize())
			{
				u64 newSizeBytes = m_uploadStagingBuffer->GetSize();
				while (newSizeBytes < m_stagingBufferOffset + sizeInBytes)
				{
					newSizeBytes = static_cast<u64>(1.8f * static_cast<float>(newSizeBytes));
				}
				m_uploadStagingBuffer->Resize(newSizeBytes);
			}
			// Upload the data.
			m_uploadStagingBuffer->Upload(data, sizeInBytes, m_stagingBufferOffset, 0);
			m_stagingBufferOffset += sizeInBytes;
		}
	}
}
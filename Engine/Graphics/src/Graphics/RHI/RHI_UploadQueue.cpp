#include "Graphics/RHI/RHI_UploadQueue.h"
#include "Graphics/RenderContext.h"
#include "Graphics/RHI/RHI_Resource.h"

#include "Core/Profiler.h"
#include "Platforms/Platform.h"

#include <iterator>

namespace Insight
{
	namespace Graphics
	{
		RHI_UploadQueueRequestInternal::RHI_UploadQueueRequestInternal(RHI_UploadQueueFunction function, RHI_Resource* resource, u64 sizeInBytes, u64 offset, u64 alignment)
			: UploadFunction(std::move(function))
			, Resource(resource)
			, SizeInBytes(std::move(sizeInBytes))
			, Offset(offset)
			, Alignment(alignment)
		{
			RHI_ResourceRenderTracker::Instance().TrackResource(resource);

			SizeInBytes = AlignUp(SizeInBytes, alignment);

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
				RHI_Buffer_Overrides overrides;
				overrides.Force_Host_Writeable = true;

				m_uploadStagingBuffer = Renderer::CreateRawBuffer(c_UploadBufferMaxSize, overrides);
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

		RPtr<RHI_UploadQueueRequest> RHI_UploadQueue::UploadBuffer(const void* data, u64 sizeInBytes, u64 offset, u64 alignment, RHI_Buffer* buffer)
		{
			IS_PROFILE_FUNCTION();

			RPtr<RHI_UploadQueueRequestInternal> uploadRequest = MakeRPtr<
				RHI_UploadQueueRequestInternal>(
					[this](const RHI_UploadQueueRequestInternal* request, RHI_CommandList* cmdList)
					{
						if (request->Cancelled)
						{
							return;
						}

						request->Request->Status = DeviceUploadStatus::Uploading;
						cmdList->CopyBufferToBuffer(static_cast<RHI_Buffer*>(request->Resource), request->Offset, m_uploadStagingBuffer, m_frameUploadOffset, request->SizeInBytes);
						m_frameUploadOffset += request->SizeInBytes;
					}, buffer, sizeInBytes, offset, alignment);

			UploadData(data, sizeInBytes,  RHI_UploadTypes::Buffer, uploadRequest);

			if (uploadRequest)
			{
				std::lock_guard lock(m_mutex);
				m_queuedUploads.push_back(std::move(uploadRequest));
			}

			return m_queuedUploads.back()->Request;
		}

		RPtr<RHI_UploadQueueRequest> RHI_UploadQueue::UploadTexture(const void* data, u64 sizeInBytes, RHI_Texture* texture)
		{
			IS_PROFILE_FUNCTION();

			const RHI_TextureFootprint footprint = RenderContext::Instance().GetTextureFootprint(texture->GetInfo());
			const u64 totalStagingBytesRequired = footprint.NumRows * footprint.RowPitch;

			RPtr<RHI_UploadQueueRequestInternal> uploadRequest = MakeRPtr<
				RHI_UploadQueueRequestInternal>(
					[this](const RHI_UploadQueueRequestInternal* request, RHI_CommandList* cmdList)
					{
						if (request->Cancelled)
						{
							return;
						}

						RHI_Texture* texture = static_cast<RHI_Texture*>(request->Resource);

						request->Request->Status = DeviceUploadStatus::Uploading;
						PipelineBarrier barreir;
						barreir.SrcStage = static_cast<u32>(PipelineStageFlagBits::TopOfPipe);
						barreir.DstStage = static_cast<u32>(PipelineStageFlagBits::Transfer);

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

						barreir.SrcStage = static_cast<u32>(PipelineStageFlagBits::Transfer);
						barreir.DstStage = static_cast<u32>(PipelineStageFlagBits::FragmentShader);

						imageBarrier.SrcAccessFlags = AccessFlagBits::TransferWrite;
						imageBarrier.DstAccessFlags = AccessFlagBits::ShaderRead;
						imageBarrier.OldLayout = texture->GetLayout();
						imageBarrier.NewLayout = ImageLayout::ShaderReadOnly;
						imageBarrier.SubresourceRange = ImageSubresourceRange::SingleMipAndLayer(ImageAspectFlagBits::Colour);
						imageBarrier.Image = texture;

						barreir.ImageBarriers.push_back(imageBarrier);
						cmdList->PipelineBarrier(barreir);

				}, texture, totalStagingBytesRequired, 0, 0);

			UploadData(data, totalStagingBytesRequired, RHI_UploadTypes::Texture, uploadRequest);

			if (uploadRequest)
			{
				std::lock_guard lock(m_mutex);
				m_queuedUploads.push_back(std::move(uploadRequest));
				return m_queuedUploads.back()->Request;
			}
			return { };
		}

#ifdef IS_RESOURCE_HANDLES_ENABLED
		void RHI_UploadQueue::UploadTexture(const void* data, u64 sizeInBytes, RHI_Handle<Texture> textureHandle)
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

						Texture* tex = RenderContext::Instance().GetTexture(textureHandle);

						PipelineBarrier barreir;
						barreir.SrcStage = +PipelineStageFlagBits::TopOfPipe;
						barreir.DstStage = +PipelineStageFlagBits::Transfer;

						ImageBarrier imageBarrier;
						imageBarrier.SrcAccessFlags = AccessFlagBits::None;
						imageBarrier.DstAccessFlags = AccessFlagBits::TransferWrite;
						imageBarrier.OldLayout = tex->Layout;
						imageBarrier.NewLayout = ImageLayout::TransforDst;
						imageBarrier.SubresourceRange = ImageSubresourceRange::SingleMipAndLayer(ImageAspectFlagBits::Colour);
						//imageBarrier.Image = texture;
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
						//imageBarrier.OldLayout = texture->GetLayout();
						imageBarrier.NewLayout = ImageLayout::ShaderReadOnly;
						imageBarrier.SubresourceRange = ImageSubresourceRange::SingleMipAndLayer(ImageAspectFlagBits::Colour);
						//imageBarrier.Image = texture;

						barreir.ImageBarriers.push_back(imageBarrier);
						cmdList->PipelineBarrier(barreir);

					}, nullptr, sizeInBytes));
		}
#endif

		void RHI_UploadQueue::UploadToDevice(RHI_CommandList* cmdList)
		{
			IS_PROFILE_FUNCTION();
			//ASSERT(RenderContext::Instance().IsRenderThread());
			cmdList->BeginTimeBlock("UploadToDevice");

			m_frameUploadOffset = 0;

			// Remove all completed requests from m_runningUploads.

			std::vector<RHI_UploadQueueRequestInternal*> completedRequests;
			for (auto const& uploadRequest : m_runningUploads)
			{
				if (uploadRequest->Request->Status == DeviceUploadStatus::Completed)
				{
					completedRequests.push_back(uploadRequest.Get());
					uploadRequest->Request->Resource->m_uploadStatus = DeviceUploadStatus::Completed;
					RHI_ResourceRenderTracker::Instance().TrackResource(uploadRequest->Request->Resource);

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
				m_queuedUploads[i]->CommandList = cmdList;
				m_queuedUploads[i]->CommandList->OnWorkCompleted.Bind<&RHI_UploadQueueRequestInternal::OnWorkComplete>(m_queuedUploads[i].Get());
				//m_queuedUploads[i]->Request->Resource->m_uploadStatus = DeviceUploadStatus::Uploading;
				// Call the upload functions.

				RHI_ResourceRenderTracker::Instance().TrackResource(m_queuedUploads[i]->Resource);
				m_queuedUploads[i]->UploadFunction(m_queuedUploads.at(i).Get(), cmdList);
			}
			// Move all  our requests to the running vector.
			std::move(m_queuedUploads.begin(), m_queuedUploads.end(), std::back_inserter(m_runningUploads));
			m_queuedUploads.clear();

			m_stagingBufferOffset = 0;
			cmdList->EndTimeBlock();
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

		void RHI_UploadQueue::UploadData(const void* data, u64 sizeInBytes, RHI_UploadTypes uploadType, RPtr<RHI_UploadQueueRequestInternal>& uploadRequest)
		{
			IS_PROFILE_FUNCTION();

			Lock();

			if (uploadType == RHI_UploadTypes::Texture)
			{
				//UploadTextureData(data, sizeInBytes, uploadRequest);
				//return;
			}

			if (sizeInBytes > c_UploadBufferMaxSize)
			{
				// We must allocate a temp buffer to update this data.
				/// We need a staging buffer to upload data from CPU to GPU.
				RHI_Buffer* stagingBuffer = Renderer::CreateStagingBuffer(sizeInBytes);
				stagingBuffer->Upload(data, sizeInBytes, 0, 0);
				uploadRequest->Request->Resource->m_uploadStatus = DeviceUploadStatus::Uploading;
				RHI_CommandList* cmdList = RenderContext::Instance().GetCommandListManager().GetCommandList();
				switch (uploadType)
				{
					case Insight::Graphics::RHI_UploadTypes::Buffer:
					{
						cmdList->CopyBufferToBuffer(static_cast<RHI_Buffer*>(uploadRequest->Request->Resource), stagingBuffer);

						break;
					}
					default:
					{
						FAIL_ASSERT();
						break;
					}
				}
				cmdList->Close();
				RenderContext::Instance().SubmitCommandListAndWait(cmdList);
				RenderContext::Instance().GetCommandListManager().ReturnCommandList(cmdList);
				Renderer::FreeStagingBuffer(stagingBuffer);

				uploadRequest->Request->Resource->m_uploadStatus = DeviceUploadStatus::Completed;
				uploadRequest = {};
			}
			else
			{
				UploadDataToStagingBuffer(data, sizeInBytes, uploadRequest);
			}

			Unlock();
		}

		void RHI_UploadQueue::UploadTextureData(const void* data, u64 sizeInBytes, RPtr<RHI_UploadQueueRequestInternal>& uploadRequest)
		{
			m_stagingBufferOffset = AlignUp(m_stagingBufferOffset, 512);

			FlushStagingBuffer(sizeInBytes);
			ASSERT(sizeInBytes < c_UploadBufferMaxSize);

			const RHI_TextureFootprint footprint = RenderContext::Instance().GetTextureFootprint(static_cast<RHI_Texture*>(uploadRequest->Resource)->GetInfo());

			if (footprint.RowSizeBytes != footprint.RowPitch)
			{
				u8* mappedPtr = m_uploadStagingBuffer->GetMappedData() + m_stagingBufferOffset;
				const u8* src = static_cast<const u8*>(data);

				for (u64 rowIdx = 0; rowIdx < footprint.NumRows; ++rowIdx)
				{
					u8* destRow = mappedPtr + (rowIdx * footprint.RowPitch);
					const u8* srcRow = src + (rowIdx * footprint.RowSizeBytes);

					Platform::MemCopy(destRow, srcRow, footprint.RowSizeBytes);
				}

				m_stagingBufferOffset += sizeInBytes;
			}
			else
			{
				UploadDataToStagingBuffer(data, sizeInBytes, uploadRequest);
			}
		}

		void RHI_UploadQueue::FlushStagingBuffer(u64 sizeInBytes, bool forceFlush)
		{
			if (m_stagingBufferOffset + sizeInBytes > m_uploadStagingBuffer->GetSize() || forceFlush)
			{
				IS_PROFILE_SCOPE("Flush upload queue");
				// First flush the current data waiting to be uploaded.
				RHI_CommandList* cmdList = RenderContext::Instance().GetCommandListManager().GetCommandList();
				UploadToDevice(cmdList);
				cmdList->Close();
				RenderContext::Instance().SubmitCommandListAndWait(cmdList);
				RenderContext::Instance().GetCommandListManager().ReturnCommandList(cmdList);
			}
		}

		void RHI_UploadQueue::UploadDataToStagingBuffer(const void* data, u64 sizeInBytes, RPtr<RHI_UploadQueueRequestInternal>& uploadRequest)
		{
			FlushStagingBuffer(sizeInBytes);

			// Upload the data.
			m_uploadStagingBuffer->Upload(data, sizeInBytes, m_stagingBufferOffset, uploadRequest->Alignment);
			m_stagingBufferOffset += sizeInBytes;
		}
	}
}
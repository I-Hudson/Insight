#if defined(IS_DX12_ENABLED)

#include "Graphics/RHI/DX12/StagingDescriptorHeap.h"
#include "Graphics/RHI/DX12/RenderContext_DX12.h"
#include "Graphics/RHI/DX12/DX12Utils.h"

namespace Insight
{
	namespace Graphics
	{
		namespace RHI::DX12
		{
			DescriptorHeap::DescriptorHeap(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE heapType, u32 numDescriptors, bool isReferencedByShader)
			{
                mHeapType = heapType;
                mMaxDescriptors = numDescriptors;
                mIsReferencedByShader = isReferencedByShader;

                D3D12_DESCRIPTOR_HEAP_DESC heapDesc;
                heapDesc.NumDescriptors = mMaxDescriptors;
                heapDesc.Type = mHeapType;
                heapDesc.Flags = mIsReferencedByShader ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
                heapDesc.NodeMask = 0;

                ThrowIfFailed(device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&mDescriptorHeap)));

                mDescriptorHeapCPUStart = mDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

                if (mIsReferencedByShader)
                {
                    mDescriptorHeapGPUStart = mDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
                }

                mDescriptorSize = device->GetDescriptorHandleIncrementSize(mHeapType);
			}

			DescriptorHeap::~DescriptorHeap()
			{
                mDescriptorHeap->Release();
                mDescriptorHeap = NULL;
			}



            StagingDescriptorHeap::StagingDescriptorHeap()
                : DescriptorHeap(nullptr, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 0, false)
            { }

            StagingDescriptorHeap::StagingDescriptorHeap(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE heapType, u32 numDescriptors)
                : DescriptorHeap(device, heapType, numDescriptors, false)
            {
                mCurrentDescriptorIndex = 0;
                mActiveHandleCount = 0;
            }

            StagingDescriptorHeap::~StagingDescriptorHeap()
            {
                if (mActiveHandleCount != 0)
                {
                    throw std::logic_error("There were active handles when the descriptor heap was destroyed. Look for leaks.");
                }

                mFreeDescriptors.clear();
            }

            DescriptorHeapHandle StagingDescriptorHeap::GetNewHeapHandle()
            {
                u32 newHandleID = 0;

                if (mCurrentDescriptorIndex < mMaxDescriptors)
                {
                    newHandleID = mCurrentDescriptorIndex;
                    mCurrentDescriptorIndex++;
                }
                else if (mFreeDescriptors.size() > 0)
                {
                    newHandleID = mFreeDescriptors.back();
                }
                else
                {
                    throw std::logic_error("Ran out of dynamic descriptor heap handles, need to increase heap size.");
                }

                DescriptorHeapHandle newHandle;
                D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = mDescriptorHeapCPUStart;
                cpuHandle.ptr += newHandleID * mDescriptorSize;
                newHandle.SetCPUHandle(cpuHandle);
                newHandle.SetHeapIndex(newHandleID);
                mActiveHandleCount++;

                return newHandle;
            }

            void StagingDescriptorHeap::FreeHeapHandle(DescriptorHeapHandle handle)
            {
                mFreeDescriptors.push_back(handle.GetHeapIndex());

                if (mActiveHandleCount == 0)
                {
                    throw std::logic_error("Freeing heap handles when there should be none left");
                }
                mActiveHandleCount--;
            }


            StagingDescriptorHeapManager::~StagingDescriptorHeapManager()
            {
                m_heaps.clear();
            }

            DescriptorHeapHandle StagingDescriptorHeapManager::GetNewHeapHandle(DescriptorType type)
            {
                if (m_heaps.find(type) == m_heaps.end())
                {
                    m_heaps.emplace(type, StagingDescriptorHeap(m_context->GetDevice(), DescriptorTypeToDX12(type), 256));
                    StagingDescriptorHeap& heap = m_heaps[type];
                    return heap.GetNewHeapHandle();
                }
                return m_heaps.find(type)->second.GetNewHeapHandle();
            }

            void StagingDescriptorHeapManager::FreeHeapHandle(DescriptorType type, DescriptorHeapHandle handle)
            {
            }
        }
	}
}

#endif // if defined(IS_DX12_ENABLED)
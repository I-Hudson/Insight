#include "Asset/AssetAsyncRequest.h"
#include "Core/Asserts.h"

namespace Insight
{
    namespace Runtime
    {
        AssetAsyncRequest::AssetAsyncRequest(Ref<Asset> Asset)
            : m_asset(Asset)
            , m_requestState(New<RequestState>())
        {
        }
        AssetAsyncRequest::AssetAsyncRequest(Ref<Asset> Asset, const bool isReady)
            : m_asset(Asset)
            , m_requestState(New<RequestState>())
        {
            m_requestState->IsReady = isReady;
        }

        AssetAsyncRequest::AssetAsyncRequest(AssetAsyncRequest&& other)
        {
            *this = std::move(other);
        }

        AssetAsyncRequest::~AssetAsyncRequest()
        {
            Delete(m_requestState);
        }

        void AssetAsyncRequest::Wait() const
        {
            if (!IsReady())
            {
                std::unique_lock lk(m_cvLock);
                m_cv.wait(lk, [this]() { return IsReady(); });
            }
        }

        void AssetAsyncRequest::SetIsReady()
        {
            ASSERT(m_requestState);
            m_requestState->IsReady.store(true, std::memory_order_release);
            m_cv.notify_all();
        }

        AssetAsyncRequest& AssetAsyncRequest::operator=(AssetAsyncRequest&& other)
        {
            m_requestState = other.m_requestState;
            m_asset = std::move(other.m_asset);

            other.m_requestState = nullptr;
            other.m_asset = nullptr;

            return *this;
        }

        bool AssetAsyncRequest::IsReady() const
        {
            ASSERT(m_requestState);
            return m_requestState->IsReady.load(std::memory_order_acquire);
        }

        Ref<Asset> AssetAsyncRequest::GetAsset() const
        {
            if (m_requestState->IsReady.load(std::memory_order_acquire))
            {
                return m_asset;
            }

            return Ref<Asset>();
        }
    }
}
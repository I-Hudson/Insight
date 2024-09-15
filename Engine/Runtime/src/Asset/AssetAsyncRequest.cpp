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

        void AssetAsyncRequest::SetIsReady()
        {
            ASSERT(m_requestState);
            m_requestState->IsReady = true;
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
            return m_requestState->IsReady;
        }

        Ref<Asset> AssetAsyncRequest::GetAsset() const
        {
            ASSERT(m_asset);
            return m_asset;
        }
    }
}
#pragma once

#include "Core/ReferencePtr.h"
#include "Asset/Asset.h"

#include <condition_variable>
#include <mutex>

namespace Insight
{
    namespace Runtime
    {
        class AssetAsyncRequest : public Core::RefCount
        {
        public:
            AssetAsyncRequest() = default;
            AssetAsyncRequest(Ref<Asset> Asset);
            AssetAsyncRequest(Ref<Asset> Asset, const bool isReady);
            AssetAsyncRequest(const AssetAsyncRequest&) = delete;
            AssetAsyncRequest(AssetAsyncRequest&& other);
            ~AssetAsyncRequest();

            AssetAsyncRequest& operator=(const AssetAsyncRequest&) = delete;
            AssetAsyncRequest& operator=(AssetAsyncRequest&& other);

            bool IsReady() const;
            Ref<Asset> GetAsset() const;

            template<typename T, std::enable_if_t<std::is_base_of_v<Asset, T>, int> = 0>
            Ref<T> GetAsset() const
            {
                return m_asset.As<T>();
            }

            void Wait() const;

        private:
            void SetIsReady();

        private:
            /// @brief Store the state of a 'AssetAsyncRequest'.
            struct RequestState
            {
                std::atomic<bool> IsReady = false;
            };

            Ref<Asset> m_asset = nullptr;
            RequestState* m_requestState = nullptr;

            mutable std::mutex m_cvLock;
            mutable std::condition_variable m_cv;

            friend AssetRegistry;
        };
    }
}
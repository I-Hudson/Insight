#pragma once

#include "Core/TypeAlias.h"
#include "Core/Defines.h"
#include "Core/Singleton.h"

#include <unordered_map>
#include <vector>
#include <set>
#include <functional>
#include <mutex>

namespace Insight
{
	namespace Graphics
	{
		class RHI_Resource;

		class RHI_ResourceRenderTrackerFrame
		{
			u64 FrameCount = 0;
			std::vector<RHI_Resource*> Resoruces;
		};

		/// @brief Class to track which RHI_Resources are being used for rendering and 
		/// to release them when needed.
		class RHI_ResourceRenderTracker : public Core::Singleton<RHI_ResourceRenderTracker>
		{
			THREAD_SAFE

			using TrackedResourceMap = std::unordered_map<const RHI_Resource*, u64>;
			using DeferedReleaseFunc = std::function<void()>;
			using DeferedResourceReleaseMap = std::unordered_map<u64, std::vector<DeferedReleaseFunc>>;

		public:

			/// @brief 
			void BeginFrame();
			/// @brief Relase old resources.
			void EndFrame();

			/// @brief Track a resource for the current frame from the render graph.
			/// @param resource 
			void TrackResource(const RHI_Resource* resource);
			bool IsResourceInUse(const RHI_Resource* resource) const;
			void AddDeferedRelase(DeferedReleaseFunc release_func);

			/// @brief Release all resources. This should only be called once at the end of the execution.
			void Release();

		private:
			mutable std::mutex m_lock;
			TrackedResourceMap m_tracked_resources;
			DeferedResourceReleaseMap m_defered_resources_to_release;

			constexpr static u8 c_FrameDelay = 4;
			bool m_releaseAllResources = false;
		};
	}
}
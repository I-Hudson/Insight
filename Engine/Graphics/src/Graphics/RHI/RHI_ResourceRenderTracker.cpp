#include "Graphics/RHI/RHI_ResourceRenderTracker.h"
#include "Graphics/RHI/RHI_Resource.h"

#include "Graphics/RenderContext.h"

namespace Insight
{
	namespace Graphics
	{
		void RHI_ResourceRenderTracker::BeginFrame()
		{
			std::lock_guard lock(m_lock);
		}

		void RHI_ResourceRenderTracker::EndFrame()
		{
			std::lock_guard lock(m_lock);

			const u64 current_frame = RenderContext::Instance().GetFrameCount();
			const u64 magicFrameCount = 6;
			if (current_frame <= (RenderContext::Instance().GetFramesInFligtCount() - magicFrameCount))
			{
				// Not enough frames have passed.
				return;
			}

			const u64 out_of_date_frame = current_frame - RenderContext::Instance().GetFramesInFligtCount() - magicFrameCount;
			std::vector<u64> frams_to_remove;

			for (auto pair : m_defered_resources_to_release)
			{
				if (pair.first > out_of_date_frame)
				{
					// Frames are too new.
					break;
				}

				frams_to_remove.push_back(pair.first);
				for (auto release_func : pair.second)
				{
					release_func();
				}
			}

			for (u64 remove_frame : frams_to_remove)
			{
				m_defered_resources_to_release.erase(remove_frame);
			}
		}

		void RHI_ResourceRenderTracker::TrackResource(const RHI_Resource* resource)
		{
			if (!resource)
			{
				return;
			}
			std::lock_guard lock(m_lock);
			m_tracked_resources[resource] = RenderContext::Instance().GetFrameCount();
		}

		bool RHI_ResourceRenderTracker::IsResourceInUse(const RHI_Resource* resource) const
		{
			std::lock_guard lock(m_lock);

			if (m_releaseAllResources)
			{
				// All resources should be released no matter what.
				return false;
			}

			const u64 current_frame = RenderContext::Instance().GetFrameCount();
			// Get the latest frame this resource was used in.
			if (current_frame <= RenderContext::Instance().GetFramesInFligtCount())
			{
				// Not enough frames have passed.
				return true;
			}

			if (TrackedResourceMap::const_iterator itr = m_tracked_resources.find(resource); itr != m_tracked_resources.end())
			{
				const u64 frame_resource_offset = current_frame - itr->second;
				return frame_resource_offset > c_FrameDelay ? false : true;
			}
			return false;
		}

		void RHI_ResourceRenderTracker::AddDeferedRelase(const DeferedReleaseFunc release_func)
		{
			const u64 current_frame = RenderContext::Instance().GetFrameCount();
			std::lock_guard lock(m_lock);
			m_defered_resources_to_release[current_frame].push_back(release_func);
		}

		void RHI_ResourceRenderTracker::Release()
		{
			std::lock_guard lock(m_lock);

			m_releaseAllResources = true;
			for (auto& [frame, releaseFuncs] : m_defered_resources_to_release)
			{
				for (auto& func : releaseFuncs)
				{
					func();
				}
			}
			m_defered_resources_to_release.clear();
		}
	}
}
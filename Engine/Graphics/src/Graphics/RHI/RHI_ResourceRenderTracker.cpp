#include "Graphics/RHI/RHI_ResourceRenderTracker.h"
#include "Graphics/RHI/RHI_Resource.h"

#include "Graphics/RenderContext.h"

namespace Insight
{
	namespace Graphics
	{
		void RHI_ResourceRenderTracker::BeginFrame()
		{

		}

		void RHI_ResourceRenderTracker::EndFrame()
		{
			const u64 current_frame = RenderContext::Instance().GetFrameCount();
			if (current_frame <= RenderContext::Instance().GetFramesInFligtCount())
			{
				// Not enough frames have passed.
				return;
			}

			const u64 out_of_date_frame = current_frame - RenderContext::Instance().GetFramesInFligtCount() - 6;
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
			m_tracked_resources[resource] = RenderContext::Instance().GetFrameCount();
		}

		bool RHI_ResourceRenderTracker::IsResourceInUse(const RHI_Resource* resource) const
		{
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
				return frame_resource_offset > RenderContext::Instance().GetFramesInFligtCount() ? false : true;
			}
			return false;
		}

		void RHI_ResourceRenderTracker::AddDeferedRelase(const DeferedReleaseFunc release_func)
		{
			const u64 current_frame = RenderContext::Instance().GetFrameCount();
			m_defered_resources_to_release[current_frame].push_back(release_func);
		}

		void RHI_ResourceRenderTracker::Release()
		{
			m_releaseAllResources = true;
		}
	}
}
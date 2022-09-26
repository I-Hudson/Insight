#include "ECS/Components/TagComponent.h"

namespace Insight
{
	namespace ECS
	{
		TagComponent::TagComponent()
		{
			m_tags.push_back("Default");
			m_removeable = false;
		}

		TagComponent::~TagComponent()
		{ }
	}
}
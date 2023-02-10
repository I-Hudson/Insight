#include "ECS/Components/TagComponent.h"

namespace Insight
{
	namespace ECS
	{
		TagComponent::TagComponent()
		{
			m_tags.push_back("Default");
			m_removeable = false;
			m_allow_multiple = false;
		}

		TagComponent::~TagComponent()
		{ }

		IS_SERIALISABLE_CPP(TagComponent)
	}
}
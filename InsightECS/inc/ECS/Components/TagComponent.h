#pragma once

#include "ECS/Entity.h"

namespace Insight
{
	namespace ECS
	{
		class TagComponent : public Component
		{
		public:
			TagComponent();
			~TagComponent();

			void AddTag(std::string tag)					{ m_tags.push_back(std::move(tag)); }
			void AddTags(std::vector<std::string> tags)		{ m_tags.insert(m_tags.end(), tags.begin(), tags.end()); }
			void RemoveTag(std::string const& tag)			{ auto a =std::remove_if(m_tags.begin(), m_tags.end(), [&tag](std::string const& t) { return tag == t; }); }
			void RemoveTags(std::vector<std::string> tags)	{ std::for_each(tags.begin(), tags.end(), [this](std::string const& tag) { RemoveTag(tag); }); }
			std::vector<std::string> GetAllTags() const		{ return m_tags; }

		private:
			std::vector<std::string> m_tags;
		};
	}
}
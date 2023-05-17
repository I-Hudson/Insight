#pragma once

#include "ECS/Entity.h"
#include <algorithm>

namespace Insight
{
	namespace ECS
	{
		class TagComponent : public Component
		{
		public:
			IS_COMPONENT(TagComponent);

			/// @brief Store a vector of tags.
			TagComponent();
			virtual ~TagComponent() override;

			/// @brief Add a new tag.
			/// @param tag 
			void AddTag(std::string tag)					{ m_tags.push_back(std::move(tag)); }
			/// @brief Add multiple tags.
			/// @param tags 
			void AddTags(std::vector<std::string> tags)		{ m_tags.insert(m_tags.end(), tags.begin(), tags.end()); }
			/// @brief Remove a single tag.
			/// @param tag 
			void RemoveTag(std::string const& tag)			{ auto a = std::remove_if(m_tags.begin(), m_tags.end(), [&tag](std::string const& t) { return tag == t; }); }
			/// @brief Remote multiple tags.
			/// @param tags 
			void RemoveTags(std::vector<std::string> tags)	{ std::for_each(tags.begin(), tags.end(), [this](std::string const& tag) { RemoveTag(tag); }); }

			/// @brief Return all tags.
			/// @return std::vector<std::string>
			std::vector<std::string>& GetAllTags()					{ return m_tags; }
			const std::vector<std::string>& GetAllTags() const		{ return m_tags; }

			IS_SERIALISABLE_H(TagComponent)

		private:
			std::vector<std::string> m_tags;
		};
	}

	OBJECT_SERIALISER(ECS::TagComponent, 2,
		SERIALISE_VECTOR_PROPERTY(std::string, m_tags, 1, 0)
		SERIALISE_BASE(ECS::Component, 2, 0)
	);
}
#pragma once

#include "ECS/Entity.h"

namespace Insight
{
	namespace ECS
	{
		class TagComponent : public Component
		{
		public:

			/// @brief Store a vector of tags.
			TagComponent();
			~TagComponent();

			static constexpr char* Type_Name = "TagComponent";
			virtual const char* GetTypeName() override { return Type_Name; }

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
			std::vector<std::string> GetAllTags() const		{ return m_tags; }

		private:
			std::vector<std::string> m_tags;
		};
	}
}
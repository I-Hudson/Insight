#pragma once

#include "Insight/Core.h"
#include "Insight/Enums/PropertyFlags.h"
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include <glm/glm.hpp>
#include <imgui_internal.h>

namespace Insight
{
	namespace Editor
	{
		inline bool IS_API InternalDrawElement(const std::unordered_map<IS_PropertyFlags, std::function<bool()>>& flagFunctions, const PropertyFlags& flags)
		{
			for (auto& func : flagFunctions)
			{
				if (func.first & (IS_PropertyFlags)flags)
				{
					return func.second();
				}
			}
			IS_CORE_ERROR("'UI flag/s' are not supported for the function called.");
			return false;
		}

		constexpr inline IS_PropertyFlags IS_API InernalResolveFlags(uint32_t flags)
		{
			return (IS_PropertyFlags)flags;
		}

		/*
		/
		/ Single items
		/
		*/
		inline bool IS_API DrawBool(const std::string& name, bool* v, PropertyFlags flags = IS_PropertyFlags::ShowInEditor)
		{
			return InternalDrawElement({ {IS_PropertyFlags::ShowInEditor, [&]() { return ImGui::Checkbox(name.c_str(), v); }},
									   }, flags);
		}

		inline bool IS_API DrawInt(const std::string& name, int* v, int clampMax = 0, const std::string& format = "%d", PropertyFlags flags = IS_PropertyFlags::ShowInEditor)
		{
			return InternalDrawElement({ {InernalResolveFlags(IS_PropertyFlags::ShowInEditor | IS_PropertyFlags::ReadOnly), [&]() { return ImGui::InputInt(name.c_str(), v, 0, 0, ImGuiInputTextFlags_ReadOnly); }},
										 {IS_PropertyFlags::ShowInEditor, [&]() { return ImGui::DragInt(name.c_str(), v, 1.0f, 0, clampMax, format.c_str()); }},
									   }, flags);
		}

		inline bool IS_API DrawFloat(const std::string& name, float* v, float clampMax = 0, const std::string& format = "%.3f", PropertyFlags flags = IS_PropertyFlags::ShowInEditor)
		{
			return InternalDrawElement({ {InernalResolveFlags(IS_PropertyFlags::ShowInEditor | IS_PropertyFlags::ReadOnly), [&]() { return ImGui::InputFloat(name.c_str(), v, 0.0f, 0.0f, format.c_str(), ImGuiInputTextFlags_ReadOnly); }},
										 {IS_PropertyFlags::ShowInEditor, [&]() { return ImGui::DragFloat(name.c_str(), v, 1.0f, 0, clampMax, format.c_str()); }},
									   }, flags);
		}

		inline bool IS_API DrawString(const std::string& name, std::string* v, PropertyFlags flags = IS_PropertyFlags::ShowInEditor)
		{
			return InternalDrawElement({ {InernalResolveFlags(IS_PropertyFlags::ShowInEditor | IS_PropertyFlags::ReadOnly), [&]() { return ImGui::InputText(name.c_str(), v, ImGuiInputTextFlags_ReadOnly); }},
										 {InernalResolveFlags(IS_PropertyFlags::ShowInEditor | IS_PropertyFlags::TextBold), [&]() { ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]); bool r = ImGui::InputText(name.c_str(), v); ImGui::PopFont(); return r; }},
										 {IS_PropertyFlags::ShowInEditor, [&]() { return ImGui::InputText(name.c_str(), v); }},
									   }, flags);
		}

		inline bool IS_API DrawString(const std::string& name, PropertyFlags flags = IS_PropertyFlags::ShowInEditor)
		{
			return InternalDrawElement({ {InernalResolveFlags(IS_PropertyFlags::ShowInEditor | IS_PropertyFlags::ReadOnly), [&]() { ImGui::Text(name.c_str(), ImGuiInputTextFlags_ReadOnly); return 0; }},
										 {InernalResolveFlags(IS_PropertyFlags::ShowInEditor | IS_PropertyFlags::TextBold), [&]() { ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]); ImGui::Text(name.c_str()); ImGui::PopFont(); return 0; }},
										 {IS_PropertyFlags::ShowInEditor, [&]() { ImGui::Text(name.c_str()); return 0; }},
									   }, flags);

		}

		inline bool IS_API DrawVector(const std::string& name, const int& vecSize, float floatArray[4], const float& floatMax = 0.0f)
		{
			bool r = false;

			ImGui::Columns(2);
			ImGui::SetColumnWidth(0, 100.0f);
			ImGui::Text(name.c_str());
			ImGui::NextColumn();

			ImGui::PushMultiItemsWidths(vecSize, ImGui::CalcItemWidth());
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0,0 });

			float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
			ImVec2 buttonSize{ lineHeight + 3.0f, lineHeight };

			if (ImGui::Button("X", buttonSize))
			{
				floatArray[0] = 0.0f;
			}
			ImGui::SameLine();
			r |= ImGui::DragFloat("##X", &floatArray[0], 0.1f, 0.0f, floatMax);
			ImGui::PopItemWidth();
			ImGui::SameLine();

			if (ImGui::Button("Y", buttonSize))
			{
				floatArray[1] = 0.0f;
			}
			ImGui::SameLine();
			r |= ImGui::DragFloat("##Y", &floatArray[1], 0.1f, 0.0f, floatMax);
			ImGui::PopItemWidth();

			if (vecSize > 2)
			{
				ImGui::SameLine();
				if (ImGui::Button("Z", buttonSize))
				{
					floatArray[2] = 0.0f;
				}
				ImGui::SameLine();
				r |= ImGui::DragFloat("##Z", &floatArray[2], 0.1f, 0.0f, floatMax);
				ImGui::PopItemWidth();
			}

			if (vecSize > 3)
			{
				ImGui::SameLine();
				if (ImGui::Button("W", buttonSize))
				{
					floatArray[3] = 0.0f;
				}
				ImGui::SameLine();
				r |= ImGui::DragFloat("##W", &floatArray[3], 0.1f, 0.0f, floatMax);
				ImGui::PopItemWidth();
			}

			ImGui::PopStyleVar();
			ImGui::Columns(1);
			return r;
		}
		inline bool IS_API DrawSingleRowMatrix2(glm::mat2* mat2, const int& rowIndex, const std::string& propertyName, const std::string& rowName, const PropertyFlags& flags)
		{
			float row[2] = { (*mat2)[rowIndex].x, (*mat2)[rowIndex].y, };
			std::string label = propertyName + rowName;
			return InternalDrawElement({ {InernalResolveFlags(IS_PropertyFlags::ShowInEditor | IS_PropertyFlags::ReadOnly), [&]() { return ImGui::InputFloat2(label.c_str(), row, "%.3f", ImGuiInputTextFlags_ReadOnly); }},
										  {IS_PropertyFlags::ShowInEditor, [&]()
											{
												bool r = ImGui::DragFloat2(label.c_str(), row, 1.0f, 0.0f, (flags & IS_PropertyFlags::ClampZero) == 1 ? std::numeric_limits<float>::max() : 0.0f);
												(*mat2)[rowIndex].x = row[0]; (*mat2)[rowIndex].y = row[1];
												return r;
											}
										  },
									   }, flags);
		}
		inline bool IS_API DrawSingleRowMatrix3(glm::mat3* mat3, const int& rowIndex, const std::string& propertyName, const std::string& rowName, const PropertyFlags& flags)
		{
			float row[3] = { (*mat3)[rowIndex].x, (*mat3)[rowIndex].y, (*mat3)[rowIndex].z };
			std::string label = propertyName + rowName;
			return InternalDrawElement({ {InernalResolveFlags(IS_PropertyFlags::ShowInEditor | IS_PropertyFlags::ReadOnly), [&]() { return ImGui::InputFloat3(label.c_str(), row, "%.3f", ImGuiInputTextFlags_ReadOnly); }},
										 {IS_PropertyFlags::ShowInEditor, [&]()
											{
												bool r = ImGui::DragFloat3(label.c_str(), row, 1.0f, 0.0f, (flags & IS_PropertyFlags::ClampZero) == 1 ? std::numeric_limits<float>::max() : 0.0f);
												(*mat3)[rowIndex].x = row[0]; (*mat3)[rowIndex].y = row[1]; (*mat3)[rowIndex].z = row[2];
												return r;
											}
										 },
									   }, flags);
		}
		inline bool IS_API DrawSingleRowMatrix4(glm::mat4* mat4, const int& rowIndex, const std::string& propertyName, const std::string& rowName, const PropertyFlags& flags)
		{
			float row[4] = { (*mat4)[rowIndex].x, (*mat4)[rowIndex].y, (*mat4)[rowIndex].z, (*mat4)[rowIndex].w };
			std::string label = propertyName + rowName;
			return InternalDrawElement({ {InernalResolveFlags(IS_PropertyFlags::ShowInEditor | IS_PropertyFlags::ReadOnly), [&]() { return ImGui::InputFloat4(label.c_str(), row, "%.3f", ImGuiInputTextFlags_ReadOnly); }},
										 {IS_PropertyFlags::ShowInEditor, [&]()
											{
												bool r = ImGui::DragFloat4(label.c_str(), row, 1.0f, 0.0f, (flags & IS_PropertyFlags::ClampZero) == 1 ? std::numeric_limits<float>::max() : 0.0f);
												(*mat4)[rowIndex].x = row[0]; (*mat4)[rowIndex].y = row[1]; (*mat4)[rowIndex].z = row[2]; (*mat4)[rowIndex].w = row[3];
												return r;
											}
										 },
									   }, flags);
		}

		inline bool IS_API DrawMat2(const std::string& name, glm::mat2* v, PropertyFlags flags = IS_PropertyFlags::ShowInEditor)
		{
			bool r = false;
			r |= DrawSingleRowMatrix2(v, 0, name, " X", flags);
			r |= DrawSingleRowMatrix2(v, 1, name, " Y", flags);
			return r;
		}
		inline bool IS_API DrawMat3(const std::string& name, glm::mat3* v, PropertyFlags flags = IS_PropertyFlags::ShowInEditor)
		{
			bool r = false;
			r |= DrawSingleRowMatrix3(v, 0, name, " X", flags);
			r |= DrawSingleRowMatrix3(v, 1, name, " Y", flags);
			r |= DrawSingleRowMatrix3(v, 2, name, " Z", flags);
			return r;
		}
		inline bool IS_API DrawMat4(const std::string& name, glm::mat4* v, PropertyFlags flags = IS_PropertyFlags::ShowInEditor)
		{
			bool r = false;
			r |= DrawSingleRowMatrix4(v, 0, name, " X", flags);
			r |= DrawSingleRowMatrix4(v, 1, name, " Y", flags);
			r |= DrawSingleRowMatrix4(v, 2, name, " Z", flags);
			r |= DrawSingleRowMatrix4(v, 3, name, " W", flags);
			return r;
		}
	}
}
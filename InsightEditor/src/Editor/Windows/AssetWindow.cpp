#include "Editor/Windows/AssetWindow.h"
#include "Engine/FileSystem/FileSystem.h"

namespace Insight::Editor
{
	AssetWindow::AssetWindow(const Module::EditorModule* editorModule)
		: EditorWindow(editorModule)
		, m_assetRootPath("./", false, false)
	{
		m_filterMode = FilterMode_Files | FilterMode_Dirs;
		m_validateFile = false;
		m_showHidden = false;
		m_isDir = false;
		m_filterDirty = true;
		m_initRead = true;

		m_layout = ColumnLayout::OneColumn;
		m_colItemsLimit = 12;
		m_selectedIdx = -1;
		m_selectedExtIdx = 0;
		m_extBoxWidth = -1.0f;
		m_colWidth = 280.0f;

		m_selectedFn = "";
		m_selectedPath = "";
		m_searchInput = '\0';
	}

	void AssetWindow::Update(const float& deltaTime)
	{
		IS_PROFILE_FUNCTION();

		ImGui::Begin("Asset Browser");

		bool showError = false;

		if (m_initRead)
		{
			showError |= !(ReadAssetDirectory("./"));
			m_initRead = false;
		}

		showError |= RenderNavAndSearchBarRegion();
		showError |= RenderFileListRegion(m_layout);
		showError |= RenderInputTextAndExtRegion();
		showError |= RenderButtonsAndCheckboxRegion();

		ImGui::End();
	}

	bool AssetWindow::ReadAssetDirectory(const std::string& projectPath)
	{
		m_assetRootPath.Path = projectPath + "data";

		if (!m_assetRootPath.Path.empty())
		{
			// If we are on Windows and current path is relative then get absolute path from dirent structure
			if (!m_assetRootPath.Path.is_absolute())
			{
				std::string absolute_path = std::filesystem::absolute(m_assetRootPath.Path).string();
				std::replace(absolute_path.begin(), absolute_path.end(), '\\', '/');
				m_assetRootPath.Path = absolute_path;
			}

			for (auto& f : std::filesystem::directory_iterator(m_assetRootPath.Path))
			{
				m_assetRootPath.SubPaths.push_back(f.path());
			}
			m_assetRootPath.Sort();
		}
		else
		{
			return false;
		}
		return true;
	}

	bool AssetWindow::AlphaSortComparator(const std::filesystem::path& a, const std::filesystem::path& b)
	{
		std::string str1 = a.string();
		std::string str2 = b.string();

		const char* cc1 = str1.c_str();
		const char* cc2 = str2.c_str();

		int ca, cb;
		do
		{
			ca = (unsigned char)*cc1++;
			cb = (unsigned char)*cc2++;
			ca = std::tolower(std::toupper(ca));
			cb = std::tolower(std::toupper(cb));
		} while (ca == cb && ca != '\0');
		if (ca < cb)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	void AssetWindow::DrawFolderTree(const std::filesystem::path& path, const u32& flags)
	{
		for (auto& f : std::filesystem::directory_iterator(path))
		{
			std::string strPath = f.path().filename().string();
			std::string treeTitle;
			if (f.is_directory())
			{
				treeTitle = m_expandedPaths[strPath] ? ICON_FA_FOLDER_OPEN : ICON_FA_FOLDER;
			}
			else if (f.is_regular_file())
			{
				treeTitle = ICON_FA_FILE;
			}

			treeTitle += "  " + strPath;
			u32 newFlags = flags | (f.is_regular_file() ? ImGuiTreeNodeFlags_Leaf : ImGuiTreeNodeFlags_None);
			if (ImGui::TreeNodeEx(strPath.c_str(), newFlags,
								  treeTitle.c_str()))
			{
				m_expandedPaths[strPath] = true;
				if (!f.is_regular_file())
				{
					DrawFolderTree(f, flags);
				}
				ImGui::TreePop();
			}
			else
			{
				m_expandedPaths[strPath] = false;
			}
		}
	}

	bool AssetWindow::RenderNavAndSearchBarRegion()
	{
		return false;
	}

	bool AssetWindow::RenderFileListRegion(const ColumnLayout& layout)
	{
		ImGuiStyle& style = ImGui::GetStyle();
		ImVec2 pw_size = ImGui::GetWindowSize();
		bool show_error = false;

		if (layout == ColumnLayout::OneColumn)
		{
			ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
			ImGui::TreePush("TreeAssets");
			std::string treeTitle = (m_assetRootPath.Expanded ? ICON_FA_FOLDER_OPEN : ICON_FA_FOLDER);
			treeTitle += "  AssetsFolder";

			if (ImGui::TreeNodeEx("AssetFolder", flags, treeTitle.c_str()))
			{
				m_assetRootPath.Expanded = true;
				DrawFolderTree(m_assetRootPath.Path, flags);
				ImGui::TreePop();
			}
			else
			{
				m_assetRootPath.Expanded = false;
			}

			ImGui::TreePop();
		}
		if (layout == ColumnLayout::TwoColumn)
		{
			//Reinitialize the limit on number of selectables in one column based on height
			//m_colItemsLimit = static_cast<int>(std::max(1.0f, window_content_height / list_item_height));
			//int num_cols = static_cast<int>(std::max(1.0f, std::ceil(static_cast<float>(m_subDirs.size() + m_subFiles.size()) / m_colItemsLimit)));

			////Limitation by ImGUI in 1.75. If columns are greater than 64 readjust the limit on items per column and recalculate number of columns
			//if (num_cols > 64)
			//{
			//    int exceed_items_amount = (num_cols - 64) * m_colItemsLimit;
			//    m_colItemsLimit += static_cast<int>(std::ceil(exceed_items_amount / 64.0));
			//    num_cols = static_cast<int>(std::max(1.0f, std::ceil(static_cast<float>(m_subDirs.size() + m_subFiles.size()) / m_colItemsLimit)));
			//}

			//float content_width = num_cols * m_colWidth;
			//if (content_width < min_content_size)
			//    content_width = 0;

			//ImGui::SetNextWindowContentSize(ImVec2(content_width, 0));
			//ImGui::BeginChild("##ScrollingRegion", ImVec2(0, window_height), true, ImGuiWindowFlags_HorizontalScrollbar);
			//ImGui::Columns(num_cols);

			////Output directories in yellow
			//ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.882f, 0.745f, 0.078f, 1.0f));
			//int items = 0;
			//for (int i = 0; i < m_subDirs.size(); i++)
			//{
			//    if (!m_subDirs[i].IsHidden || m_showHidden)
			//    {
			//        items++;
			//        if (ImGui::Selectable(m_subDirs[i].Name.c_str(), m_selectedIdx == i && m_isDir, ImGuiSelectableFlags_AllowDoubleClick))
			//        {
			//            m_selectedIdx = i;
			//            m_isDir = true;

			//            if (ImGui::IsMouseDoubleClicked(0))
			//            {
			//                show_error |= !(OnDirClick(i));
			//                break;
			//            }
			//        }
			//        if ((items) % m_colItemsLimit == 0)
			//        {
			//            ImGui::NextColumn();
			//        }
			//    }
			//}
			//ImGui::PopStyleColor(1);

			////Output files
			//for (int i = 0; i < m_subFiles.size(); i++)
			//{
			//    if (!m_subFiles[i].IsHidden || m_showHidden)
			//    {
			//        items++;
			//        if (ImGui::Selectable(m_subFiles[i].Name.c_str(), m_selectedIdx == i && !m_isDir, ImGuiSelectableFlags_AllowDoubleClick))
			//        {
			//            //int len = filtered_files[i]->name.length();
			//            m_selectedIdx = i;
			//            m_isDir = false;

			//            if (ImGui::IsMouseDoubleClicked(0))
			//            {
			//                m_selectedFn = m_subFiles[i].Name;
			//                m_validateFile = true;
			//            }
			//        }
			//        if ((items) % m_colItemsLimit == 0)
			//        {
			//            ImGui::NextColumn();
			//        }
			//    }
			//}
			//ImGui::Columns(1);
			//ImGui::EndChild();
		}

		return show_error;
	}

	bool AssetWindow::RenderInputTextAndExtRegion()
	{
		return false;
	}

	bool AssetWindow::RenderButtonsAndCheckboxRegion()
	{
		return false;
	}

	bool AssetWindow::OnDirClick(const int& idx)
	{
		return false;
	}
}
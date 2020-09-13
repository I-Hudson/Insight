#include "ispch.h"

#include "Insight/Renderer/FrameGraph/FrameGraph.h"
#include "Insight/Log.h"

namespace Insight
{
	namespace FrameGraph
	{
		FrameGraph::FrameGraph()
		{
		}

		FrameGraph::~FrameGraph()
		{
			Destroy();
		}

		void FrameGraph::Create()
		{
			m_rootNode = FGNode();

			m_rootNode.Parent = nullptr;
			m_rootNode.Children = std::vector<FGNode>();
			m_rootNode.Command = nullptr;
			m_rootNode.FrameGraph = this;
			m_rootNode.OwnerObject = nullptr;
			m_rootNode.OwnerObjectType = "";
			m_rootNode.DebugText = "Root Node";

			m_allNodes.push_back(m_rootNode);
		}

		void FrameGraph::Destroy()
		{
		}

		void FrameGraph::Execute()
		{
			ExecuteNode(&m_rootNode);
		}

		void FrameGraph::AddNode(void* parentObject, std::function<void()> func, void* ownerObject, const std::string& ownerObjectType,
			const bool& allowChildren, const std::string& debugText)
		{
			FGNode& parent = m_rootNode;

			FGNode newNode;
			newNode.Parent = &parent;
			newNode.Children = std::vector<FGNode>();
			newNode.Command = func;
			newNode.FrameGraph = this;
			newNode.OwnerObject = ownerObject;
			newNode.OwnerObjectType = ownerObjectType;
			newNode.AllowChildren = allowChildren;
			newNode.DebugText = debugText;

			m_allNodes.push_back(newNode);
			parent.Children.push_back(newNode);
		}

		void FrameGraph::RemoveNode(void* ptr)
		{
			FGNode node;
			for (auto it = m_allNodes.begin(); it != m_allNodes.end(); ++it)
			{
				if (it->OwnerObject == ptr)
				{
					node = *it;
					m_allNodes.erase(it);
					break;
				}
			}

			for (auto it = node.Parent->Children.begin(); it != node.Parent->Children.end(); ++it)
			{
				if (it->OwnerObject == node.OwnerObject)
				{
					node.Parent->Children.erase(it);
					break;
				}
			}
			
			for (auto it = node.Children.begin(); it != node.Children.end(); ++it)
			{
				it->Parent = node.Parent;
			}
		}


		void FrameGraph::PrintToConsole()
		{
#ifdef IS_DEBUG
			int indentWidth = 0;
			PrintNode(&m_rootNode, indentWidth);
#endif
		}

#ifdef IS_DEBUG
		void FrameGraph::PrintNode(FGNode* node, int indentWidth)
		{
			int indent = indentWidth > 0 ? indentWidth - 1 : 0;

			std::string output = std::string(indent, ' ') + "|" + node->OwnerObjectType;
			IS_CORE_INFO("{0}", output);
			output = std::string(indent, ' ') + "|" + node->DebugText;
			IS_CORE_INFO("{0}", output);

			bool removeIndent = false;
			for (auto it = node->Children.begin(); it != node->Children.end(); ++it)
			{
				if (!removeIndent)
				{
					indentWidth += 4;
				}
				PrintNode(&(*it), indentWidth);
				removeIndent = true;
			}

			if (removeIndent)
			{
				indentWidth -= 4;
			}
		}
#endif

		void FrameGraph::ExecuteNode(FGNode* node)
		{
			if (node->Command != nullptr)
			{
				node->Command();
			}

			for (auto it = node->Children.begin(); it != node->Children.end(); ++it)
			{
				ExecuteNode(&(*it));
			}
		}

	}
}
#pragma once

#include "Engine/Core/Core.h"

	namespace FrameGraph
	{
		class FrameGraph;

		struct FGNode
		{
			FGNode* Parent;
			std::vector<FGNode> Children;
			std::function<void()> Command;
			FrameGraph* FrameGraph;
			void* OwnerObject;
			std::string OwnerObjectType;
			bool AllowChildren;
			std::string DebugText;
		};

		class FrameGraph
		{
		public:
			FrameGraph();
			~FrameGraph();

			void Create();
			void Destroy();

			void Execute();

			void AddNode(void* parentObject, std::function<void()> func, void* ownerObject, const std::string& ownerObjectType, 
				const bool& allowChildren = true, const std::string& debugText = "");
			void RemoveNode(void* ptr);

			void PrintToConsole();

		private:
#ifdef IS_DEBUG
			void PrintNode(FGNode* node, int indentWidth);
#endif

			void ExecuteNode(FGNode* node);

			FGNode m_rootNode;
			std::vector<FGNode> m_allNodes;
		};
	}
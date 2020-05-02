#pragma once

#include "Insight/Core.h"

namespace Insight
{
	namespace Memory
	{
		template <class T>
		class SinglyLinkedList
		{
		public:
			struct Node
			{
				T data;
				Node* next;
			};

			Node* Head;

			SinglyLinkedList();

			void Insert(Node* previousNode, Node* newNode);

			void Remove(Node* previousNode, Node* deleteNode);
		};

		template<class T>
		inline SinglyLinkedList<T>::SinglyLinkedList()
		{

		}

		template<class T>
		inline void SinglyLinkedList<T>::Insert(Node* previousNode, Node* newNode)
		{
			if (previousNode == nullptr)
			{
				// Is the first node
				if (Head != nullptr) 
				{
					// The list has more elements
					newNode->next = Head;
				}
				else
				{
					newNode->next = nullptr;
				}
				Head = newNode;
			}
			else 
			{
				if (previousNode->next == nullptr)
				{
					// Is the last node
					previousNode->next = newNode;
					newNode->next = nullptr;
				}
				else
				{
					// Is a middle node
					newNode->next = previousNode->next;
					previousNode->next = newNode;
				}
			}
		}

		template<class T>
		inline void SinglyLinkedList<T>::Remove(Node* previousNode, Node* deleteNode)
		{
			if (previousNode == nullptr)
			{
				// Is the first node
				if (deleteNode->next == nullptr)
				{
					// List only has one element
					Head = nullptr;
				}
				else
				{
					// List has more elements
					Head = deleteNode->next;
				}
			}
			else
			{
				previousNode->next = deleteNode->next;
			}
		}
	}
}





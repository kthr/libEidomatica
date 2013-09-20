/*
 * circularLinkedList.hpp
 *
 *  Created on: Jul 10, 2013
 *      Author: kthierbach
 */

#ifndef CIRCULARLINKEDLIST_HPP_
#define CIRCULARLINKEDLIST_HPP_

#include "listNode.hpp"

namespace elib
{

template <class T>
class CircularLinkedList
{
	public:
		CircularLinkedList()
		{
		}
		virtual ~CircularLinkedList()
		{
			ListNode<T> *next;
			actualElement = head;
			while(!isEmpty())
			{
				next = actualElement->next;
				delete actualElement;
				--numberOfElements;
				actualElement = next;
			}
		}

		bool isEmpty()
		{
			return (numberOfElements == 0);
		}
		int size()
		{
			return numberOfElements;
		}
		void addLast(T data)
		{
			if (isEmpty())
			{
				ListNode<T> *listNode = new ListNode<T>(data, head);
				head = listNode;
				tail = listNode;
				numberOfElements++;
				actualElement = head;
			}
			else
			{
				ListNode<T> *listNode = new ListNode<T>(data, nullptr);
				tail->next = listNode;
				tail = listNode;
				numberOfElements++;
			}
		}
		T* getActualElementData()
		{
			if (!isEmpty())
			{
				if (actualElement == nullptr)
				{
					actualElement = head;
				}
				return actualElement->data;
			}
			return nullptr;
		}
		T* getNext()
		{
			if (isEmpty())
				return nullptr;
			index = (index + 1) % numberOfElements;
			if (index == 0)
				actualElement = head;
			else
				actualElement = actualElement->next;
			return actualElement->data;
		}
		bool setActualElement(T data)
		{
			int tmpIndex = index;
			if (data == *(actualElement->data))
				return true;
			index = (index + 1) % numberOfElements;
			if (index == 0)
				actualElement = head;
			else
				actualElement = actualElement->next;
			while ((index != tmpIndex))
			{
				if (data == *(actualElement->data))
					return true;
				index = (index + 1) % numberOfElements;
				if (index == 0)
					actualElement = head;
				else
					actualElement = actualElement->next;
			}
			return false;
		}

	private:
		ListNode<T> *head = nullptr,
					*tail = nullptr,
					*actualElement = nullptr;
		int numberOfElements = 0;
		int index = 0;
};

} /* namespace elib */
#endif /* CIRCULARLINKEDLIST_HPP_ */

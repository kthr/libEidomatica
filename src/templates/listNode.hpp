/*
 * listNode.hpp
 *
 *  Created on: Jul 10, 2013
 *      Author: kthierbach
 */

#ifndef LISTNODE_HPP_
#define LISTNODE_HPP_

namespace elib
{

template <class T>
class ListNode
{

	public:
		ListNode(T& data, ListNode<T> *head)
		:head(head), data(new T(data))
		{
		}
		~ListNode()
		{
			delete data;
		}
		ListNode<T> *head = nullptr,
					*next = nullptr;
		T* data;

};

} /* namespace elib */
#endif /* LISTNODE_HPP_ */

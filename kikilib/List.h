//@Author Liu Yukang 
#pragma once
#include <memory>
#include "ObjPool.h"

namespace kikilib
{
	template<class ValType>
	class ListNode
	{
	public:
		ListNode(ValType oval) : val(oval), next(nullptr) {};
		~ListNode() {};
		ValType val;
		ListNode* next;
	};

	template<class ItValType>
	class ListIterator
	{
	public:

		using iterator_category = std::forward_iterator_tag;
		using value_type = ItValType;
		using difference_type = ptrdiff_t;
		using pointer = value_type*;
		using reference = value_type&;

		ListIterator() : _ptr(nullptr) {};

		ListIterator(ListNode<ItValType>* ptr) : _ptr(ptr) {};

		~ListIterator() {};

		ListIterator(ListIterator& oit) { _ptr = oit._ptr; };
		ListIterator(ListIterator&& oit) { _ptr = oit._ptr; };

		ListIterator& operator=(const ListIterator& oit)
		{
			_ptr = oit._ptr;
			return *this;
		};

		ListIterator& operator=(ListIterator&& oit)
		{
			_ptr = oit._ptr;
			return *this;
		};

		ItValType& operator*() const
		{
			return _ptr->val;
		};

		ItValType* operator->() const {
			return &(_ptr->val);
		};

		ListIterator& operator++() {
			_ptr = _ptr->next;
			return *this;
		};

		ListIterator operator++(int) {
			ListIterator tmp = *this;
			_ptr = _ptr->next;
			return tmp;
		};

		bool operator==(const ListIterator& right) const {
			return _ptr == right._ptr;
		};

		bool operator!=(const ListIterator& right) const {
			return !(_ptr == right._ptr);
		};

	private:
		ListNode<ItValType>* _ptr;
	};

	//实验证明使用迭代器效率慢一倍
	template<class ValType>
	class List
	{
	public:

		using  MyValType = ValType;
		using  iterator = ListIterator<MyValType>;

		List() : _head(nullptr), _tail(nullptr), _size(0) {};
		~List()
		{
			ListNode<ValType>* tmp = nullptr;
			while (_head)
			{
				tmp = _head;
				_head = _head->next;
				delete tmp;
			}
		};

		void push_back(ValType val)
		{
			++_size;
			if (!_tail)
			{
				//auto tmp = new ListNode<ValType>(val);
				auto tmp = _listNodePool.New(val);
				_head = tmp;
				_tail = tmp;
			}
			else
			{
				//_tail->next = new ListNode<ValType>(val);
				_tail->next = _listNodePool.New(val);
				_tail = _tail->next;
			}
		};

		void pop_front()
		{
			--_size;
			if (!_head)
			{
				throw("List no data when pop_front()!");
			}
			else
			{
				auto tmp = _head;
				if (_head == _tail)
				{
					_head = nullptr;
					_tail = nullptr;
				}
				else
				{
					_head = _head->next;
				}
				//delete tmp;
				_listNodePool.Delete(tmp);
			}
		};

		ListNode<ValType>* GetHead()
		{
			return _head;
		};

		ListNode<ValType>* GetTail()
		{
			return _tail;
		};

		iterator begin()
		{
			return iterator(_head);
		};

		iterator back_Iter()
		{
			return iterator(_tail);
		};

		iterator end()
		{
			return iterator(nullptr);
		};

		ValType& front()
		{
			if (!_head)
			{
				throw("List no data when front()!");
			}
			return _head->val;
		};

		ValType& back()
		{
			if (!_head)
			{
				throw("List no data when back()!");
			}
			return _tail->val;
		};

		bool empty()
		{
			return (!_head);
		};

		long long size()
		{
			return _size;
		}

	private:
		ListNode<ValType>* _head;
		ListNode<ValType>* _tail;
		long long _size;
		ObjPool<ListNode<ValType>> _listNodePool;
	};
}
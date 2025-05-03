#ifndef DOUBLE_CIRCULAR_LINK_LIST_H
#define DOUBLE_CIRCULAR_LINK_LIST_H

#include"../../memory_pool/my_memory_pool.h"
#include<iostream>
#include<memory>
#include<iomanip>
#include<exception>


using namespace my_memory_pool;

//带头节点的双向循环链表，同时有尾结点
template<class T>
struct LinkNode
{
	T value;
	struct LinkNode* next;
	struct LinkNode* prev;

	LinkNode() = default;
	explicit LinkNode(const T& value) : value(value), next(nullptr), prev(nullptr){}
	LinkNode(const T& value, struct LinkNode* next, struct LinkNode* prev) 
		: value(value), next(next), prev(prev){}
};


template<class T, class Alloc = std::allocator<T> >
class DoubleCircularLinkList
{
	using Node = LinkNode<T>;
	//使用自定义内存池中的分配器接口
	typedef typename Alloc::template rebind<Node>::other allocator;
private:
	
	Node* head_;
	Node* tail_;
	size_t size_;
	allocator allocator_;

public:

	//默认 且初始状态时 head_ = tail_
	DoubleCircularLinkList()
		:size_(0)
	{
		head_ = allocator_.allocate(1);
		allocator_.construct(head_, T(), nullptr, nullptr);
		head_->next = head_->prev = head_;
		tail_ = head_;
	}
	
	~DoubleCircularLinkList()
	{
		clear();
	}

public:
	bool empty() const {return size_ == 0;} // 
	
	size_t size() const {return size_;} 

	//以下参数中的 pos 		取值范围： [1, size_ + 1]
	//以下参数中的 index 	取值范围： [1，size_]
	//（1:代表头节点后第一个节点位置，即首元节点；size_ + 1:代表尾结点后位置）
	//------------------------- 插入 ------------------------	
	
	//在合法任意位置 pos 处插入 
	Node* insert(size_t pos, const T& elem)
	{
		return insert_aux(pos, elem);
	}

	//头插法
	Node* insert_from_head(const T& elem)
	{
		return insert(1, elem);
	}

	//尾插法
	Node* insert_from_tail(const T& elem)
	{
		return insert(size_ + 1, elem);
	}

	//------------------------- 删除 ------------------------
	
	//删除合任意位置 index 处的元素
	Node* remove(size_t index)
	{
		return remove_aux(index);
	}
	
	//头删法
	Node* remove_from_head()
	{
		return remove(1);
	}

	//尾删法
	Node* remove_from_tail()
	{
		return remove(size_);
	}

	//清空函数
	void clear()
	{
		//调用该函数，当然需要!empty()，由于调用的是 头删法，已经包含判空操作
		//assert(size_ != 0);
		while(!empty())
		{
			remove_from_head();
		}
	}

	//------------------------- 修改 ------------------------
	
	Node* update(size_t index, const T& elem)
	{
		return update_aux(index, elem);
	}

	//------------------------- 查找 ------------------------
	
	T get_head_value()
	{
		return node_value(1);
	}

	T get_tail_value()
	{
		return node_value(size_); 
	}

	T get_node_value(size_t index)
	{
		return node_value(index);
	}
	
	//------------------------- 打印 ------------------------

	void print() const
	{
		print_aux();
	}

private:
	
	//内部辅助函数
	
	//empty_exception，当删除等需要判空等操作时，throw异常
	void empty_exception()
	{
		if(empty()) throw std::out_of_range("链表为空的，待插入元素......");
	}

	//------------------------- 插入 ------------------------
	
	Node* insert_aux(size_t pos, const T& elem)
	{
		if(pos <= 0 || pos > size_ + 1) throw std::out_of_range("插入位置不合法......");
		//找到插入位置前一个位置的节点

		Node* newNode = allocator_.allocate(1);
		allocator_.construct(newNode, elem, nullptr, nullptr);

		if(pos == 1)
		{
			newNode->next = head_->next;
			newNode->prev = head_;
			head_->next->prev = newNode;
			head_->next = newNode;
		}
		else if(pos == size_ + 1)
		{
			newNode->next = head_;
			head_->prev = newNode;
			tail_->next = newNode;
			newNode->prev = tail_;
			tail_ = newNode;
		}
		else
		{
			//找到插入节点前一个节点
			Node* preNode = find_node(pos - 1);
			newNode->next = preNode->next;
			newNode->prev = preNode;
			preNode->next->prev = newNode;
			preNode->next = newNode;
		}

		//当size_ == 0时插入插入节点即为新的尾结点
		if(size_ == 0) tail_ = newNode;
		++size_;
		return newNode;
	}

	//------------------------- 删除 ------------------------
	
	Node* remove_aux(size_t index)
	{
		empty_exception();
		if(index <= 0 || index > size_) throw std::out_of_range("删除位置不合法......");

		Node* node = find_node(index);
		Node* target = node;
		node->next->prev = node->prev;
		node->prev->next = node->next;

		if(index == 1) head_->next = node->next;
		if(index == size_) tail_ = node->prev;

		allocator_.destroy(node);
		allocator_.deallocate(node, 1);
		size_--;
		return target;
	}

	//------------------------- 修改 ------------------------
	
	Node* update_aux(size_t index, const T& elem)
	{
		Node* node = find_node(index);
		node->value = elem;
		return node;
	}

	//------------------------- 查找 ------------------------
	
	Node* find_node(size_t index)
	{
		if(index <= 0 || index > size_) throw std::out_of_range("节点位置不合法......");
		if(index == size_) return tail_;
		Node* cur = head_->next;
		for(int i = 1;i < index;i++)
		{
			cur = cur->next;
		}
		return cur;
	}

	T node_value(size_t index)
	{
		return find_node(index)->value;
	}
	
	//------------------------- 打印 ------------------------

	void print_aux() const
	{
		if(empty()) 
		{
			std::cout << "链表为空.....," << std::endl;
			return;
		}
		std::cout << "链表规模：" << size_ << std::endl;
		std::cout << "头节点 ⇄ ";
		Node* cur = head_->next;
		for(int i = 1;i < size_;i++)
		{
			std::cout << cur->value << " ⇄ ";
			cur = cur->next;
		}
		std::cout << tail_->value << std::endl;
		std::cout << "         ↑ |" << std::setfill('_') << std::setw(5 * (size_ - 1)) << "↑ |" << std::endl;
		std::cout << "         |" << std::setfill('_') << std::setw(5 * (size_ - 1)) << "|" << std::endl << std::endl;
	}
};


#endif

#ifndef MY_MEMORY_POOL_H
#define MY_MEMORY_POOL_H

#include"type_traits.h"
#include<new>
#include<cstddef>
#include<cassert>
#include<utility>

namespace my_memory_pool
{


	template<class T, size_t BlockSize = 4096>
		class memory_pool
		{
		public:
			using value_type 								= 						T;
			using reference 								= 						T&;
			using pointer 									=		 				T*;
			using const_pointer 							= 						const T*;
			using const_reference 							= 						const T&;
			using size_type 								= 						std::size_t;
			using difference_type 							= 						std::ptrdiff_t;


			//用于指示在容器进行拷贝赋值操作时，分配器是否会从原容器传播到目标容器
			//默认为 false_type ：目标容器不会改变其分配器，仍然使用原有的分配器
			using propagate_on_container_copy_assignment 	= 						m_false_type;

			//用于指示在容器进行移动赋值操作时，分配器是否会从原容器传播到目标容器
			//默认为 true_type : 目标容器在移动赋值时会获得原容器的分配器实例
			using propagate_on_container_move_assignment 	= 						m_true_type;

			//用于指示在容器进行交换操作时，分配器是否应该被交换
			//默认为 true_type : 容器在交换时会同时交换它们原本的分配器
			using propagate_on_container_swap 				= 						m_true_type;
			
			//分配器接口中的一个标准机制，用于将当前分配器类型重新绑定到另一种类型，允许分配器在容器中适配不容类型的对象
			template<class U> 
				struct rebind
				{
					using other = memory_pool<U>;
				};

			//construct / deconstruct
			memory_pool() noexcept;
			memory_pool(const memory_pool&) = delete;
			memory_pool(memory_pool&&) noexcept;
			//template<class U> memory_pool(const memory_pool<U>&) noexcept;
			
			~memory_pool() noexcept;

			memory_pool& operator=(const memory_pool& other) = delete;
			memory_pool& operator=(memory_pool&& other) noexcept;

			pointer allocate(size_type n = 1, const_pointer hint = nullptr);
			void deallocate(pointer ptr, size_type n = 1);

			template<class U, class... Args>
				void construct(U* ptr, Args&&... args);

			template<class U>
				void destroy(U* ptr);

			size_type max_size() const noexcept;
			
			

		private:
			
			//内存单元
			union slot_
			{
				value_type element;
				slot_* next;
			};

			using slot_type = slot_;

			//当前内存块
			slot_* current_block_;
			//空闲（内存单元）链表
			slot_* free_slots_;
			//块数
			size_type block_count_;

			//分配一个新的内存块，并将其链接到 
			void allocate_block();
			void reset_pool() noexcept;


			//对齐函数：将一个给定的地址或大小(size)调整为指定的对齐边界的倍数(align)
			static constexpr size_type align_up(size_type size, size_type align) noexcept
			{
				//返回对齐后的值，满足 返回值 % align == 0
				//原理：将 低log2(align) 置零
				return (size + (align - 1)) & ~(align - 1);
			}
			
			static_assert(BlockSize >= 2 * sizeof(slot_), "BlockSize too small");
		};

	template<class T, size_t BlockSize>
	memory_pool<T, BlockSize>::memory_pool() noexcept
	:current_block_(nullptr), free_slots_(nullptr), block_count_(0)
	{
	}

	template<class T, size_t BlockSize>
	memory_pool<T, BlockSize>::memory_pool(memory_pool&& other) noexcept
	:current_block_(other.current_block_), free_slots_(other.free_slots_),
	block_count_(other.block_count_)
	{
		other.current_block_ = nullptr;
		other.free_slots_ = nullptr;
		other.block_count_ = 0;
	}

	template<class T, size_t BlockSize>
	memory_pool<T, BlockSize>& memory_pool<T, BlockSize>::operator=(memory_pool&& other) noexcept
	{
		if(this != &other)
		{
			reset_pool();
			current_block_ = other.current_block_;
			free_slots_ = other.free_slots_;
			block_count_ = other.block_count_;
			other.current_block_ = nullptr;
			other.free_slots_ = nullptr;
			other.block_count_ = 0;
		}
		return *this;
	}

	template<class T, size_t BlockSize>
	memory_pool<T, BlockSize>::~memory_pool() noexcept
	{
		reset_pool();
	}


	template<class T, size_t BlockSize>
	void memory_pool<T, BlockSize>::reset_pool() noexcept
	{
		while(current_block_)
		{
			slot_* next = current_block_->next;
			::operator delete(current_block_);
			current_block_ = next;
		}
		free_slots_ = nullptr;
		block_count_ = 0;
	}

	template<class T, size_t BlockSize>
	typename memory_pool<T, BlockSize>::pointer memory_pool<T, BlockSize>::allocate(size_type n, const_pointer)
	{
		assert(n == 1 && "MemoryPool can only allocate one object at a time");

		//如果空闲链表中有空闲内存单元
		if(free_slots_)
		{
			slot_* result = free_slots_;
			free_slots_ = free_slots_->next;
			return reinterpret_cast<pointer>(result);
		}

		//如果当前块为空或 
		//(reinterpret_cast<char*>的目的：指针的运算是以字节为单位进行操作的)
		if(!current_block_ || reinterpret_cast<char*>(free_slots_) + sizeof(slot_) > reinterpret_cast<char*>(current_block_) + BlockSize)
		{
			allocate_block();
		}

		slot_* result = free_slots_;
		free_slots_ = free_slots_->next;
		return reinterpret_cast<pointer>(result);
	}

	template<class T, size_t BlockSize>
	void memory_pool<T, BlockSize>::deallocate(pointer ptr, size_type)
	{
		if(ptr)
		{
			slot_* slot = reinterpret_cast<slot_*>(ptr);
			//头插法
			slot->next = free_slots_;
			free_slots_ = slot;
		}
	}


	template<class T, size_t BlockSize>
		template<class U, class... Args>
	void memory_pool<T, BlockSize>::construct(U* ptr, Args&&... args)
	{
		new (ptr) U(std::forward<Args>(args)...);
	}

	template<class T, size_t BlockSize>
		template<class U>
	void memory_pool<T, BlockSize>::destroy(U* ptr)
	{
		ptr->~U();
	}

	template<class T, size_t BlockSize>
	typename memory_pool<T, BlockSize>::size_type memory_pool<T, BlockSize>::max_size() const noexcept
	{
		return BlockSize / sizeof(slot_);
	}

	template<class T, size_t BlockSize>
	void memory_pool<T, BlockSize>::allocate_block()
	{
		size_type block_size = memory_pool<T, BlockSize>::align_up(BlockSize, alignof(slot_));
		slot_* new_block = reinterpret_cast<slot_*>(::operator new(block_size));
		new_block->next = current_block_;
		current_block_ = new_block;

		char* body = reinterpret_cast<char*>(new_block) + sizeof(slot_);
		size_type body_padding = align_up(reinterpret_cast<size_type>(body), alignof(slot_)) - reinterpret_cast<size_type>(body);
		free_slots_ = reinterpret_cast<slot_*>(body + body_padding);

		slot_* last_slot = reinterpret_cast<slot_*>(reinterpret_cast<char*>(new_block) + block_size - sizeof(slot_));
		for(slot_* slot = free_slots_;slot < last_slot;++slot)
		{
			slot->next = slot + 1;
		}
		last_slot->next = nullptr;

		++block_count_;
	}
	


};//namespace MyMemoryPool


#endif//MY_MEMORY_POOL_H

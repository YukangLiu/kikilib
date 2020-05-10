//@Author Liu Yukang 
#pragma once
#include <type_traits>
#include "MemPool.h"

namespace kikilib
{

	template<class T>
	class ObjPool
	{
	public:
		ObjPool() {};
		~ObjPool() {};

		DISALLOW_COPY_MOVE_AND_ASSIGN(ObjPool);

		template<typename... Args>
		inline T* New(Args... args);

		inline void Delete(void* obj);

	private:
		template<typename... Args>
		inline T* New_aux(std::true_type, Args... args);

		template<typename... Args>
		inline T* New_aux(std::false_type, Args... args);

		inline void Delete_aux(std::true_type, void* obj);

		inline void Delete_aux(std::false_type, void* obj);

		MemPool<sizeof(T)> _memPool;

	};

	template<class T>
	template<typename... Args>
	inline T* ObjPool<T>::New(Args... args)
	{
		return New_aux(std::integral_constant<bool, std::is_trivially_constructible<T>::value>(), args...);
	}

	template<class T>
	template<typename... Args>
	inline T* ObjPool<T>::New_aux(std::true_type, Args... args)
	{
		return static_cast<T*>(_memPool.AllocAMemBlock());
	}

	template<class T>
	template<typename... Args>
	inline T* ObjPool<T>::New_aux(std::false_type, Args... args)
	{
		void* newPos = _memPool.AllocAMemBlock();
		return new(newPos) T(args...);
	}

	template<class T>
	inline void ObjPool<T>::Delete(void* obj)
	{
		if (!obj)
		{
			return;
		}
		Delete_aux(std::integral_constant<bool, std::is_trivially_destructible<T>::value>(), obj);
	}

	template<class T>
	inline void ObjPool<T>::Delete_aux(std::true_type, void* obj)
	{
		_memPool.FreeAMemBlock(obj);
	}

	template<class T>
	inline void ObjPool<T>::Delete_aux(std::false_type, void* obj)
	{
		(static_cast<T*>(obj))->~T();
		_memPool.FreeAMemBlock(obj);
	}
	
}
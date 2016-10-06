#pragma once

#include "ObjectPool.hpp"

class TypePool : ObjectPool{
	static int _typeCounter;

	template <typename T>
	inline uint64_t _typeId();

public:
	TypePool(uint64_t chunkSize);

	template <typename T>
	inline uint64_t insert(const T& data);

	template <typename T>
	inline void set(uint64_t index, const T& data);

	template <typename T>
	inline T* get(uint64_t index);

	template <typename T>
	inline void remove(uint64_t index);
};

int TypePool::_typeCounter = 0;

template<typename T>
inline uint64_t TypePool::_typeId(){
	static int type = _typeCounter++;
	return type;
}

TypePool::TypePool(uint64_t chunkSize) : ObjectPool(chunkSize){}

template<typename T>
inline uint64_t TypePool::insert(const T & data){
	uint64_t index = ObjectPool::insert(sizeof(T));
	std::memcpy(ObjectPool::get(index), &data, sizeof(T));
	return index;
}

template<typename T>
inline void TypePool::set(uint64_t index, const T & data){
	ObjectPool::set(index, sizeof(T));
	std::memcpy((void*)ObjectPool::get(index), &data, sizeof(T));
}

template<typename T>
inline T * TypePool::get(uint64_t index){
	return (T*)ObjectPool::get(index);
}

template<typename T>
inline void TypePool::remove(uint64_t index){
	ObjectPool::remove(index);
	freeRemoved(1);
}

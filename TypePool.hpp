#pragma once

#include "ObjectPool.hpp"

class TypePool : ObjectPool{
	static int _typeCounter;

	template <typename T>
	inline uint64_t _typeId();

	void _setType(uint64_t index, uint64_t type);

	std::vector<uint64_t> _indexTypes;

public:
	TypePool(uint64_t chunkSize);

	template <typename T>
	inline uint64_t insert();

	template <typename T>
	inline uint64_t insert(const T& data);

	template <typename T>
	inline void set(uint64_t index);

	template <typename T>
	inline void set(uint64_t index, const T& data);

	template <typename T>
	inline T* get(uint64_t index);

	template <typename T>
	inline bool has(uint64_t index);

	inline void remove(uint64_t index);

	inline void clear();
};

int TypePool::_typeCounter = 1;

template<typename T>
inline uint64_t TypePool::_typeId(){
	static int type = _typeCounter++;
	return type;
}

inline void TypePool::_setType(uint64_t index, uint64_t type){
	if (index >= _indexTypes.size())
		_indexTypes.resize(index + 1);

	_indexTypes[index] = type;
}

TypePool::TypePool(uint64_t chunkSize) : ObjectPool(chunkSize){}

template<typename T>
inline uint64_t TypePool::insert(){
	uint64_t index = ObjectPool::insert(sizeof(T));

	_setType(index, _typeId<T>());

	return index;
}


template<typename T>
inline uint64_t TypePool::insert(const T & data){
	uint64_t index = insert<T>();

	std::memcpy(ObjectPool::get(index), &data, sizeof(T));

	return index;
}

template<typename T>
inline void TypePool::set(uint64_t index){
	ObjectPool::set(index, sizeof(T));

	_setType(index, _typeId<T>());
}

template<typename T>
inline void TypePool::set(uint64_t index, const T & data){
	set<T>(index);

	std::memcpy((void*)ObjectPool::get(index), &data, sizeof(T));
}

template<typename T>
inline T * TypePool::get(uint64_t index){
	if (!has<T>(index))
		return nullptr;

	return (T*)ObjectPool::get(index);
}

template<typename T>
inline bool TypePool::has(uint64_t index){
	if (!ObjectPool::has(index) && _typeId<T>() != _indexLocations[index])
		return false;

	return true;
}

inline void TypePool::remove(uint64_t index){
	if (!ObjectPool::has(index))
		return;

	ObjectPool::remove(index);
	freeRemoved(1);

	_indexLocations[index] = 0;
}

inline void TypePool::clear(){
	ObjectPool::clear();

	std::vector<uint64_t>().swap(_indexTypes);
}

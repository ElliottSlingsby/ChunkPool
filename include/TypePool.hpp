#pragma once

#include "ChunkPool.hpp"
#include "BitHelper.hpp"

#include <cstdint>
#include <functional>
#include <tuple>

#ifndef MAX_TYPES
#define MAX_TYPES 16
#endif

// TypePool: An extension of ChunkPool for storing groups of data types and iterating over them using lambdas with type pointers as parameters.
// Each block of memory allocated in the ChunkPool has a mask describing what objects that block contains, masks being automatically created from template and lambda arguments.
// The lambda iterator will only iterate over blocks containing the data types provided as pointers in the lambda parameters.

/*
pool.insert<Banana, Dog, Puzzle>(1, 1, 1);				// Will iterate over (arguments are how many of each type)
pool.insert<Banana, Dog, Wizard, Puzzle>(1, 1, 1, 5);	// Will iterate over (5 of Puzzle for array example)
pool.insert<Banana, Dog>(1, 1, 1);						// Will NOT iterate over (doesn't contain Puzzle)

pool.execute([](const TypePool::Mask& mask, Banana* banana, Dog* dog, Puzzle* puzzle){   // Data types provided as pointers in the lambda arguments (mask is a required arg for pool lambdas)	
	
	// Do stuff here
	
	for (unsigned int i = 0; i < mask.length<Puzzle(); i++){
		puzzle[i];		// Array example
	}

	return;
});
*/

class TypePool{
public:
	class Mask{
		uint8_t* _start = nullptr;

		inline bool _fits(const Mask& other);

	public:		
		template <typename T>
		inline unsigned int length() const;

		friend class TypePool;
	};

private:
	ChunkPool _pool;

	uint8_t* _maskBuffer = nullptr;
	uint32_t _maskCount = 0;
	
	//uint32_t* _versions = nullptr; // TODO: Reintegrate 64 bit IDs and versioning
	//unsigned int _versionCount = 0;

	static uint32_t _typeCounter;

	static size_t* _typeSizes;

	template <typename T>
	static inline uint32_t _typeId();

	template <unsigned int I, typename ...Args>
	inline typename std::enable_if<I == sizeof...(Args), void>::type _fillMask(const Mask& mask);

	template <unsigned int I, typename ...Args>
	inline typename std::enable_if<I <sizeof...(Args), void>::type _fillMask(const Mask& mask);

	template <unsigned int I, typename ...Args, typename ...Is>
	inline typename std::enable_if<I == sizeof...(Args), void>::type _fillMask(const Mask& mask, Is... i);

	template <unsigned int I, typename ...Args, typename ...Is>
	inline typename std::enable_if<I <sizeof...(Args), void>::type _fillMask(const Mask& mask, Is... i);

	inline void _clearMask(const Mask& mask);

	inline Mask _getMask(uint32_t id);

	template <typename ...Args>
	inline Mask _tupleMask(const std::tuple<Args*...>& tuple);

	template <typename T, typename ...Args>
	inline std::tuple<Args*...> _lambdaTuple(void(T::*lambda)(const Mask&, Args*...) const);

	template <unsigned int I, typename ...Args>
	inline typename std::enable_if<I == sizeof...(Args), void>::type _fillTuple(const Mask& mask, std::tuple<Args*...>* tuple, uint8_t* data);

	template <unsigned int I, typename ...Args>
	inline typename std::enable_if<I <sizeof...(Args), void>::type _fillTuple(const Mask& mask, std::tuple<Args*...>* tuple, uint8_t* data);

	template <typename T, typename ...Args>
	inline void _callLambda(const T& lambda, const Mask& mask, std::tuple<Args*...>* tuple);

	template <unsigned int I, typename ...Args>
	inline typename std::enable_if<I == sizeof...(Args), size_t>::type _sizeOf();

	template <unsigned int I, typename ...Args>
	inline typename std::enable_if<I <sizeof...(Args), size_t>::type _sizeOf();

	template <unsigned int I, typename ...Args, typename ...Is>
	inline typename std::enable_if<I == sizeof...(Args), size_t>::type _sizeOf(Is... i);

	template <unsigned int I, typename ...Args, typename ...Is>
	inline typename std::enable_if<I <sizeof...(Args), size_t>::type _sizeOf(Is... i);
	
	template <typename T>
	inline size_t _typeOffset(const Mask& mask);

public:
	inline TypePool(size_t chunkSize);
	inline ~TypePool();

	template <typename ...Args, typename ...Is>
	inline uint32_t insert(Is... i);

	inline void erase(uint32_t id);
	
	template <typename T>
	inline unsigned int length(uint32_t id);

	template <typename T>
	inline T* get(uint32_t id);

	template <typename T>
	inline void execute(const T& lambda);
};

inline bool TypePool::Mask::_fits(const Mask& other){
	bool fits = true;

	for (unsigned int i = 0; i < MAX_TYPES; i++){
		if (_start[i] && !other._start[i]){
			fits = false;
			break;
		}
	}

	return fits;
}

template<typename T>
inline unsigned int TypePool::Mask::length() const{
	return _start[_typeId<T>()];
}

template<typename ...Args>
inline TypePool::Mask TypePool::_tupleMask(const std::tuple<Args*...>& tuple){
	static Mask mask;

	if (!mask._start){
		mask._start = (uint8_t*)std::calloc(1, MAX_TYPES); 
		_fillMask<0, Args...>(mask);
	}

	return mask;
}

template<typename T, typename ...Args>
std::tuple<Args*...> TypePool::_lambdaTuple(void(T::*lambda)(const Mask&, Args*...) const){
	return std::tuple<Args*...>();
}

template <unsigned int I, typename ...Args>
typename std::enable_if<I == sizeof...(Args), void>::type TypePool::_fillTuple(const Mask& mask, std::tuple<Args*...>* tuple, uint8_t* data){}

template <unsigned int I, typename ...Args>
typename std::enable_if<I < sizeof...(Args), void>::type TypePool::_fillTuple(const Mask& mask, std::tuple<Args*...>* tuple, uint8_t* data){
	using T = std::tuple_element_t<I, std::tuple<Args...>>;
	std::get<I>(*tuple) = (T*)(data + _typeOffset<T>(mask));

	_fillTuple<I + 1>(mask, tuple, data);
}

template <typename T, typename ...Args>
void TypePool::_callLambda(const T& lambda, const Mask& mask, std::tuple<Args*...>* tuple){
	lambda(mask, std::get<Args*>(*tuple)...);
}

template<unsigned int I, typename ...Args>
typename std::enable_if<I == sizeof...(Args), size_t>::type TypePool::_sizeOf(){
	return 0;
}

template <unsigned int I, typename ...Args>
typename std::enable_if < I <sizeof...(Args), size_t>::type TypePool::_sizeOf(){
	using T = std::tuple_element_t<I, std::tuple<Args...>>;
	return sizeof(T) + _sizeOf<I + 1, Args...>();
}

template <unsigned int I, typename ...Args, typename ...Is>
inline typename std::enable_if<I == sizeof...(Args), size_t>::type TypePool::_sizeOf(Is... i){
	return 0;
}

template <unsigned int I, typename ...Args, typename ...Is>
inline typename std::enable_if < I < sizeof...(Args), size_t>::type TypePool::_sizeOf(Is... i){
	using T = std::tuple_element_t<I, std::tuple<Args...>>;
	return (sizeof(T) * std::get<I>(std::tuple<Is...>(i...))) + _sizeOf<I + 1, Args...>(i...);
}

template<typename T>
inline size_t TypePool::_typeOffset(const Mask& mask){
	size_t offset = 0;

	for (unsigned int i = 0; i < _typeId<T>(); i++){
		offset += _typeSizes[i] * mask._start[i];
	}

	return offset;
}

uint32_t TypePool::_typeCounter = 0;

size_t* TypePool::_typeSizes = nullptr;

template<typename T>
uint32_t TypePool::_typeId(){
	static uint32_t i = _typeCounter++;

	assert(_typeCounter < MAX_TYPES);

	if (!_typeSizes)
		_typeSizes = (size_t*)std::malloc(sizeof(size_t) * MAX_TYPES);

	static bool first = true;

	if (first){
		first = false;
		_typeSizes[i] = sizeof(T);
	}

	return i;
}

template <unsigned int I, typename ...Args>
typename std::enable_if<I == sizeof...(Args), void>::type TypePool::_fillMask(const Mask& mask){}

template <unsigned int I, typename ...Args>
typename std::enable_if<I < sizeof...(Args), void>::type TypePool::_fillMask(const Mask& mask){
	using T = std::tuple_element_t<I, std::tuple<Args...>>;
	mask._start[_typeId<T>()] = 1;

	_fillMask<I + 1, Args...>(mask);
}

template <unsigned int I, typename ...Args, typename ...Is>
inline typename std::enable_if<I == sizeof...(Args), void>::type TypePool::_fillMask(const Mask& mask, Is... i){}

template <unsigned int I, typename ...Args, typename ...Is>
inline typename std::enable_if<I < sizeof...(Args), void>::type TypePool::_fillMask(const Mask& mask, Is... i){
	using T = std::tuple_element_t<I, std::tuple<Args...>>;
	mask._start[_typeId<T>()] = std::get<I>(std::tuple<Is...>(i...));

	_fillMask<I + 1, Args...>(mask, i...);
}

inline void TypePool::_clearMask(const Mask& mask){
	std::memset(mask._start, 0, MAX_TYPES);
}

inline TypePool::Mask TypePool::_getMask(uint32_t id){
	if (id >= _maskCount){
		_maskCount = id + 1;

		if (!_maskBuffer)
			_maskBuffer = (uint8_t*)std::malloc(_maskCount * MAX_TYPES);
		else
			_maskBuffer = (uint8_t*)std::realloc(_maskBuffer, _maskCount * MAX_TYPES);
	}

	Mask mask;
	mask._start = _maskBuffer + (id * MAX_TYPES);

	return mask;
}

TypePool::TypePool(size_t chunkSize) : _pool(chunkSize){}

TypePool::~TypePool(){
	if (_maskBuffer)
		std::free(_maskBuffer);

	// TODO: Reintegrate versioning
	//if (_versions)
	//	std::free(_versions);
}

template <typename ...Args, typename ...Is>
inline uint32_t TypePool::insert(Is... i){
	uint32_t id = _pool.insert(_sizeOf<0, Args...>(i...));

	Mask mask = _getMask(id);

	_clearMask(mask);
	_fillMask<0, Args...>(mask, i...);

	return id;
}

void TypePool::erase(uint32_t id){
	_clearMask(_getMask(id));
	_pool.erase(id);
}

template<typename T>
inline unsigned int TypePool::length(uint32_t id){
	return _getMask(id)._start[_typeId<T>()];
}

template<typename T>
inline T* TypePool::get(uint32_t id){
	return (T*)(_pool.get(id) + _typeOffset<T>(_getMask(id)));
}

template<typename T>
void TypePool::execute(const T& lambda){
	auto tuple = _lambdaTuple(&T::operator());
	Mask tupleMask = _tupleMask(tuple);

	ChunkPool::Iterator iter = _pool.begin();

	while (iter.valid()){
		Mask iterMask = _getMask(iter.id());

		if (tupleMask._fits(iterMask)){
			_fillTuple<0>(iterMask, &tuple, iter.get());
			_callLambda(lambda, iterMask, &tuple);
		}

		iter.next();
	}
}
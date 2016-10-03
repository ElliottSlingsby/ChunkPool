#pragma once

#include "MemHelp.hpp"

#include <vector>
#include <queue>
#include <stack>

class ObjectPool{
protected:
	uint8_t* _buffer = nullptr;
	uint64_t _bufferSize = 0;

	uint64_t _chunkSize = 0;

	std::vector<MemHelp::Info> _indexLocations;
	std::vector<uint8_t> _indexRemoved;

	std::vector<MemHelp::Info> _freeMemory;
	std::priority_queue<MemHelp::Size> _freeMemoryQueue;

	std::vector<MemHelp::Location> _orderedIndexes;

	std::queue<uint64_t> _removedIndexes;

	inline void _expandInfoBuffers();

	inline bool _validIndex(uint64_t index) const;

	inline void _rebuildFreeMemory();
	inline void _rebuildFreeMemoryQueue();

	inline void _rebuildIndexOrder();

	inline void _returnMemory(MemHelp::Info memory, bool rebuildQueue = true);

	inline MemHelp::Info _findMemory(uint64_t size);

public:
	class Iterator{
	protected:
		ObjectPool* _pool = nullptr;
		MemHelp::Info _memory = 0;
		uint64_t _i = 0;

	public:
		Iterator(){}

		Iterator(ObjectPool* pool, MemHelp::Info memory){
			if (!pool)
				return;

			_pool = pool;
			_memory = memory;
			_i = 0;
		}

		Iterator(const Iterator& other){
			operator=(other);
		}

		virtual ~Iterator(){}

		void operator=(const Iterator& other){
			_pool = other._pool;
			_memory = other._memory;
		}

		inline bool valid() const{
			return _pool && _memory;
		}

		inline uint64_t size() const{
			if (valid())
				return _memory.size;

			return 0;
		}

		inline uint8_t* get(){
			if (valid())
				return _pool->_buffer + _memory.start;

			return nullptr;
		}

		inline void next(){
			if (!valid())
				return;

			_i++;

			if (_i < _pool->_orderedIndexes.size()){
				_memory = _pool->_orderedIndexes[_i];
			}
			else{
				_memory = 0;
				_i = 0;
			}
		}
	};

	friend class Iterator;

	ObjectPool(uint64_t chunkSize);
	virtual ~ObjectPool();

	// Returns index to new clear block of memory of size 
	inline uint64_t insert(uint64_t size);

	// Returns byte pointer to block of memory belonging to index
	inline uint8_t* get(uint64_t index);

	// Marks index as removed and queues block of memory to be freed
	inline void remove(uint64_t index);

	// Frees removed blocks of memory for re-use
	inline void freeRemoved(uint64_t limit = 0);

	// Potentially allocate memory to guerentee enough of size, or chunk size if 0
	inline void reserve(uint64_t minimum = 0);

	// Potentially de-allocate memory by size if smaller than chunk, or as much as possible if 0
	inline void shrink(uint64_t maximum = 0);

	// Clear everything, and reset all buffers
	inline void clear();

	// Set minimum size to allocate new memory
	inline void setChunkSize(uint64_t size);

	// Return iterator to first element in linear memory, or non-valid if empty
	inline Iterator begin();

	// Returns total buffer size in bytes
	inline uint64_t bufferSize() const;

	// Return amount of fragmented un-used buffer memory in bytes
	inline uint64_t gapSize() const;

	// Return amount of top un-used buffer memory in bytes
	inline uint64_t topSize() const;
};

inline void ObjectPool::_expandInfoBuffers(){
	_indexLocations.push_back(0);
	_indexRemoved.push_back(0);
}

inline bool ObjectPool::_validIndex(uint64_t index) const{
	if (index < _indexLocations.size() && _indexLocations[index] != 0 && _indexRemoved[index] == 0)
		return true;

	return false;
}

inline void ObjectPool::_rebuildFreeMemory(){
	std::priority_queue<MemHelp::Size> _queue = _freeMemoryQueue;
	_freeMemory.clear();

	while (_queue.size()){
		_freeMemory.push_back(_queue.top());
		_queue.pop();
	}
}

inline void ObjectPool::_rebuildFreeMemoryQueue(){
	_freeMemoryQueue = std::priority_queue<MemHelp::Size>();

	for (const MemHelp::Info& memory : _freeMemory){
		_freeMemoryQueue.push(memory);
	}
}

inline void ObjectPool::_rebuildIndexOrder(){
	_orderedIndexes.clear();

	for (uint64_t i = 0; i < _indexLocations.size(); i++){
		if (_validIndex(i))
			_orderedIndexes.push_back(_indexLocations[i]);
	}

	std::sort(_orderedIndexes.begin(), _orderedIndexes.end());
}

inline void ObjectPool::_returnMemory(MemHelp::Info memory, bool rebuildQueue){
	std::queue<MemHelp::Info> carryOver;

	for (const MemHelp::Info& freeLocation : _freeMemory){
		if (memory.adjacent(freeLocation))
			memory = memory.combine(freeLocation);
		else
			carryOver.push(freeLocation);
	}

	_freeMemory.clear();
	_freeMemory.push_back(memory);

	while (!carryOver.empty()){
		_freeMemory.push_back(carryOver.front());
		carryOver.pop();
	}

	if (rebuildQueue)
		_rebuildFreeMemoryQueue();
}

inline MemHelp::Info ObjectPool::_findMemory(uint64_t size){
	reserve(size);

	std::stack<MemHelp::Info> larger;

	while (_freeMemoryQueue.size() && _freeMemoryQueue.top().size >= size){
		larger.push(_freeMemoryQueue.top());
		_freeMemoryQueue.pop();
	}

	MemHelp::Info top = larger.top();
	larger.pop();

	MemHelp::Info sized(top.start, size);
	MemHelp::Info remain = top.subtract(sized);

	if (remain)
		_freeMemoryQueue.push(remain);

	while (larger.size()){
		_freeMemoryQueue.push(larger.top());
		larger.pop();
	}

	_rebuildFreeMemory();

	return sized;
}

inline ObjectPool::ObjectPool(uint64_t chunkSize){
	_chunkSize = chunkSize;
}

inline ObjectPool::~ObjectPool(){
	std::free(_buffer);
}

inline uint64_t ObjectPool::insert(uint64_t size){
	uint64_t index = 0;

	for (uint64_t i = 0; i < _indexLocations.size(); i++){
		if (!_indexLocations[i])
			index = i;
	}

	if (!index){
		_expandInfoBuffers();
		index = _indexLocations.size() - 1;
	}

	MemHelp::Info memory = _findMemory(size);

	_indexLocations[index] = memory;

	std::memset(_buffer + memory.start, 0, memory.size);

	_rebuildIndexOrder();

	return index;
}

inline uint8_t* ObjectPool::get(uint64_t index){
	if (!_validIndex(index))
		return nullptr;

	MemHelp::Info memory = _indexLocations[index];

	return _buffer + memory.start;
}

inline void ObjectPool::remove(uint64_t index){
	if (!_validIndex(index) || _indexRemoved[index])
		return;

	_indexRemoved[index] = 1;
	_removedIndexes.push(index);
}

inline void ObjectPool::freeRemoved(uint64_t limit){
	if (!limit)
		limit = _removedIndexes.size();

	uint64_t inserted = 0;

	while (inserted < limit){
		uint64_t index = _removedIndexes.front();

		_returnMemory(_indexLocations[index], false);

		_indexLocations[index] = 0;
		_indexRemoved[index] = 0;

		_removedIndexes.pop();
		inserted++;
	}

	_rebuildFreeMemoryQueue();

	_rebuildIndexOrder();
}

inline void ObjectPool::reserve(uint64_t minimum){
	MemHelp::Info top;
	
	if (_freeMemoryQueue.size() && top.end() != _bufferSize)
		top = _freeMemoryQueue.top();

	if (_chunkSize > minimum)
		minimum = _chunkSize;

	if (top.size >= minimum)
		return;

	MemHelp::Info newTop(_bufferSize, minimum - top.size);

	_returnMemory(newTop);

	_bufferSize += newTop.size;
	_buffer = MemHelp::allocate(_bufferSize, _buffer);
}

inline void ObjectPool::shrink(uint64_t maximum){
	MemHelp::Info top = _freeMemoryQueue.top();

	if (top.end() != _bufferSize || top.size < _chunkSize)
		return;

	if (!maximum)
		maximum = top.size;

	_freeMemoryQueue.pop();
	_freeMemoryQueue.push(MemHelp::Info(top.start, top.size - maximum));

	_bufferSize -= maximum;
	_buffer = MemHelp::allocate(_bufferSize, _buffer);

	_rebuildFreeMemory();
}

inline void ObjectPool::clear(){
	std::free(_buffer);
	_buffer = nullptr;

	_bufferSize = 0;

	std::vector<MemHelp::Info>().swap(_indexLocations);
	std::vector<uint8_t>().swap(_indexRemoved);
	std::vector<MemHelp::Info>().swap(_freeMemory);

	_freeMemoryQueue = std::priority_queue<MemHelp::Size>();
	_removedIndexes = std::queue<uint64_t>();
}

inline void ObjectPool::setChunkSize(uint64_t size){
	if (size)
		_chunkSize = size;
}

inline ObjectPool::Iterator ObjectPool::begin(){
	if (_orderedIndexes.empty())
		return Iterator();

	return Iterator(this, *_orderedIndexes.begin());
}

inline uint64_t ObjectPool::bufferSize() const{
	return _bufferSize;
}

inline uint64_t ObjectPool::gapSize() const{
	uint64_t empty = 0;

	for (const MemHelp::Info& free : _freeMemory){
		if (free.end() != _bufferSize)
			empty += free.size;
	}

	return empty;
}

inline uint64_t ObjectPool::topSize() const{
	if (_freeMemoryQueue.size() && _freeMemoryQueue.top().end() == _bufferSize)
		return _freeMemoryQueue.top().size;

	return 0;
}

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

	std::queue<uint64_t> _removedIndexes;
	 
	inline void _expandBuffer();
	inline void _shrinkBuffer();

	inline void _expandInfoBuffers();

	inline bool _validIndex(uint64_t index) const;

	inline void _rebuildFreeMemory();
	inline void _rebuildFreeMemoryQueue();

	inline void _returnMemory(MemHelp::Info memory, bool rebuildQueue = true);
	inline MemHelp::Info _findMemory(uint64_t size);

public:
	ObjectPool(uint64_t chunkSize);
	virtual ~ObjectPool();

	inline uint64_t insert(uint64_t size);

	inline uint8_t* get(uint64_t index);

	inline void remove(uint64_t index);

	inline void reserve(uint64_t size);

	inline void processRemoved(uint64_t limit = 0);

	inline void clear();

	inline void setChunkSize(uint64_t size);

	inline uint64_t size() const;
};

inline void ObjectPool::_expandBuffer(){
	MemHelp::Info top(_bufferSize, _chunkSize);

	_returnMemory(top);

	_bufferSize += _chunkSize;
	_buffer = MemHelp::allocate(_bufferSize, _buffer);
}

inline void ObjectPool::_shrinkBuffer(){
	MemHelp::Info top = _freeMemoryQueue.top();

	if (top.end() != _bufferSize || top.size < _chunkSize)
		return;

	_freeMemoryQueue.pop();
	_freeMemoryQueue.push(MemHelp::Info(0, top.size - _chunkSize));

	_bufferSize -= _chunkSize;
	_buffer = MemHelp::allocate(_bufferSize, _buffer);

	_rebuildFreeMemory();
}

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

	while (_freeMemoryQueue.top().size >= size){
		larger.push(_freeMemoryQueue.top());
		_freeMemoryQueue.pop();

		if (_freeMemoryQueue.empty())
			break;
	}

	MemHelp::Info top = larger.top();
	larger.pop();

	MemHelp::Info sized(top.start, size);
	MemHelp::Info remain = top.subtract(sized);

	if (remain)
		larger.push(remain);

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

inline void ObjectPool::reserve(uint64_t size){
	while (_freeMemoryQueue.empty() || _freeMemoryQueue.top().size < size){
		_expandBuffer();
	}
}

inline void ObjectPool::processRemoved(uint64_t limit){
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

	_shrinkBuffer();
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

inline uint64_t ObjectPool::size() const{
	return _bufferSize;
}

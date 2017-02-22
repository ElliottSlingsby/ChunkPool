#pragma once

#include "BitHelper.hpp"
#include "FlatStack.hpp"

#include <cstdint>
#include <cassert>
#include <cstdlib>
#include <cstring>

#include <iostream>

// ChunkPool: For creating varyingly sized blocks of pre-allocated memory while maintaining some-what contiguous memory (some-what as there's empty space at the top of each chunk).
// Each chunk gets filled with contiguous blocks of data (tied to 32bit ids), and when full, another chunk is created for more space.
// When an element is removed from a chunk, elements after it are copied over to maintain contiguous memory.
// The chunk size is what dictates performance depending on the sizes of blocks being created, as a larger chunk size means less time allocating, and smaller chunk size means less time copying.

class ChunkPool{
protected:
	struct Location{
		enum Flags{
			LeftExists,
			RightExists,
			Active
		};

		uint32_t index;
		uint32_t id;

		uint8_t flags = 0;

		size_t startSize;
		size_t endSize;

		uint32_t leftLocation;
		uint32_t rightLocation;
	};

	struct Chunk{
		size_t chunkSize;
		size_t topSize;

		Location* locations = nullptr;
		uint32_t locationCount = 0;

		uint32_t freeLocations = 0;

		uint32_t firstLocation;
		uint32_t lastLocation;
	};

	uint8_t* _buffer = nullptr;
	size_t _bufferSize = 0;

	Chunk* _chunks = nullptr;
	uint32_t _chunkCount = 0;

	const size_t _chunkSize;

	uint64_t* _ids = nullptr;
	uint32_t _idCount = 0;

	FlatStack<uint32_t> _freeIds;

	template <typename T>
	inline T* _allocate(T* location, unsigned int count);

	inline uint32_t _pushChunk();

	inline uint32_t _pushLocation(uint32_t chunkIndex, size_t size);
	inline void _eraseLocation(uint32_t chunkIndex, uint32_t locationIndex);

	inline uint8_t* _locationPointer(uint32_t chunkIndex, uint32_t locationIndex);

public:
	class Iterator{
		ChunkPool& _pool;

		uint32_t _chunkIndex;
		uint32_t _locationIndex;

		bool _valid = true;

	public:
		inline Iterator(ChunkPool& pool, uint32_t id);
		inline Iterator(ChunkPool& pool);

		inline uint8_t* get();

		inline bool valid();

		inline void next();
	};

	friend class Iterator;

	inline ChunkPool(size_t chunkSize);
	inline ~ChunkPool();

	inline uint32_t set(size_t size);
	inline uint8_t* get(uint32_t id);
	inline void erase(uint32_t id);

	inline Iterator begin();

	inline unsigned int count() const;

	inline void print() const;
};

template <typename T>
T* ChunkPool::_allocate(T* location, unsigned int count){
	if (!location)
		return (T*)std::malloc(sizeof(T) * count);

	return (T*)std::realloc(location, sizeof(T) * count);
}

uint32_t ChunkPool::_pushChunk(){
	// Allocate and setup chunk
	_chunks = _allocate(_chunks, _chunkCount + 1);

	Chunk& chunk = _chunks[_chunkCount];

	chunk = Chunk();
	chunk.chunkSize = _chunkSize;
	chunk.topSize = _chunkSize;

	_chunkCount++;

	// Add memory buffer
	_bufferSize += _chunkSize;
	_buffer = _allocate(_buffer, (unsigned int)_bufferSize);

	return _chunkCount - 1;
}

uint32_t ChunkPool::_pushLocation(uint32_t chunkIndex, size_t size){
	Chunk& chunk = _chunks[chunkIndex];

	// Find free memory or create some for new location
	if (chunk.freeLocations)
		chunk.freeLocations--;
	else
		chunk.locations = _allocate(chunk.locations, chunk.locationCount + 1);

	Location& newLoc = chunk.locations[chunk.locationCount];

	newLoc = Location();
	newLoc.index = chunk.locationCount;
	newLoc.flags = BitHelper::setBit(newLoc.flags, Location::Active, true);

	if (chunk.locationCount == 0){
		// Create first location in chunk
		chunk.firstLocation = newLoc.index;
		chunk.lastLocation = newLoc.index;

		newLoc.startSize = 0;
		newLoc.endSize = size;
	}
	else{
		// Push location to the end of existing locations in chunk
		Location& lastLoc = chunk.locations[chunk.lastLocation];

		newLoc.leftLocation = lastLoc.index;
		newLoc.flags = BitHelper::setBit(newLoc.flags, Location::LeftExists, true);

		lastLoc.rightLocation = newLoc.index;
		lastLoc.flags = BitHelper::setBit(lastLoc.flags, Location::RightExists, true);
		
		chunk.lastLocation = newLoc.index;

		newLoc.startSize = lastLoc.endSize;
		newLoc.endSize = lastLoc.endSize + size;
	}

	// Remove from chunk location count and top size
	chunk.locationCount++;
	chunk.topSize -= size;

	return newLoc.index;
}

void ChunkPool::_eraseLocation(uint32_t chunkIndex, uint32_t locationIndex){
	Chunk& chunk = _chunks[chunkIndex];

	Location& location = chunk.locations[locationIndex];

	// Update adjacent locations to remove erased location
	if (BitHelper::getBit(location.flags, Location::LeftExists) && BitHelper::getBit(location.flags, Location::RightExists)){
		Location& left = chunk.locations[location.leftLocation];
		Location& right = chunk.locations[location.rightLocation];

		left.rightLocation = right.index;
		right.leftLocation = left.index;
	}
	else if (BitHelper::getBit(location.flags, Location::LeftExists)){
		Location& left = chunk.locations[location.leftLocation];
		left.flags = BitHelper::setBit(left.flags, Location::RightExists, false);

		chunk.lastLocation = left.index;
	}
	else if (BitHelper::getBit(location.flags, Location::RightExists)){
		Location& right = chunk.locations[location.rightLocation];
		right.flags = BitHelper::setBit(right.flags, Location::LeftExists, false);

		chunk.firstLocation = right.index;
	}

	// Copy and update end location (in terms of array position) to plug the erased gap 
	Location& end = chunk.locations[chunk.locationCount - 1];

	if (end.index != location.index){
		location.flags = end.flags;
		location.startSize = end.startSize;
		location.endSize = end.endSize;
		location.id = end.id;

		// Update first and last to point to new index
		if (chunk.firstLocation == end.index)
			chunk.firstLocation = location.index;

		if (chunk.lastLocation == end.index)
			chunk.lastLocation = location.index;

		// Update adjacent locations to point to new index
		if (BitHelper::getBit(end.flags, Location::LeftExists)){
			chunk.locations[end.leftLocation].rightLocation = location.index;
			location.rightLocation = end.rightLocation;
		}

		if (BitHelper::getBit(end.flags, Location::RightExists)){
			chunk.locations[end.rightLocation].leftLocation = location.index;
			location.leftLocation = end.leftLocation;
		}

		// Update replaced location's id with new location
		_ids[end.id] = BitHelper::combine(chunkIndex, location.index);
	}

	// Add to top size and available locations
	chunk.topSize += location.endSize - location.startSize;
	chunk.locationCount--;
	chunk.freeLocations++;
}

uint8_t* ChunkPool::_locationPointer(uint32_t chunkIndex, uint32_t locationIndex){
	Location& location = _chunks[chunkIndex].locations[locationIndex];

	return _buffer + (_chunkSize * chunkIndex) + location.startSize;
}

ChunkPool::Iterator::Iterator(ChunkPool& pool, uint32_t id) : _pool(pool){
	uint64_t pair = _pool._ids[id];

	_chunkIndex = BitHelper::front(pair);
	_locationIndex = BitHelper::front(pair);
}

ChunkPool::Iterator::Iterator(ChunkPool& pool) : _pool(pool){
	_valid = false;
}

uint8_t* ChunkPool::Iterator::get(){
	if (!_valid)
		return nullptr;

	return _pool._locationPointer(_chunkIndex, _locationIndex);
}

bool ChunkPool::Iterator::valid(){
	return _valid;
}

void ChunkPool::Iterator::next(){
	if (!_valid)
		return;

	if (_locationIndex < _pool._chunks[_chunkIndex].locationCount - 1){
		_locationIndex++;
	}
	else if (_chunkIndex < _pool._chunkCount - 1){
		_chunkIndex++;

		while (!_pool._chunks[_chunkIndex].locationCount){
			if (_chunkIndex == _pool._chunkCount - 1){
				_valid = false;
				return;
			}

			_chunkIndex++;
		}

		_locationIndex = _pool._chunks[_chunkIndex].firstLocation;
	}
	else{
		_valid = false;
	}
}

ChunkPool::ChunkPool(size_t chunkSize) : _chunkSize(chunkSize){
	_pushChunk();
}

ChunkPool::~ChunkPool(){
	for (unsigned int i = 0; i < _chunkCount; i++){
		if (_chunks[i].locations)
			std::free(_chunks[i].locations);
	}

	if (_chunks)
		std::free(_chunks);

	if (_ids)
		std::free(_ids);

	if (_buffer)
		std::free(_buffer);
}

uint32_t ChunkPool::set(size_t size){
	assert(size <= _chunkSize);

	// Find first available chunk with top size big enough
	bool found = false;
	uint32_t chunkIndex = 0;

	for (; chunkIndex < _chunkCount; chunkIndex++){
		if (_chunks[chunkIndex].topSize >= size){
			found = true;
			break;
		}
	}

	// Create new chunk if none available
	if (!found){
		_pushChunk();
		chunkIndex = _chunkCount - 1;
	}

	// Push new location to chunk and clear memory in buffer
	uint32_t locationIndex = _pushLocation(chunkIndex, size);

	std::memset(_locationPointer(chunkIndex, locationIndex), 0, size);

	// Assign chunk and location to id
	uint32_t id;

	if (!_freeIds.empty()){
		id = _freeIds.top();
		_freeIds.pop();
	}
	else{
		_ids = _allocate(_ids, _idCount + 1);
		id = _idCount;
		_idCount++;
	}

	_ids[id] = BitHelper::combine(chunkIndex, locationIndex);

	// Update location with id
	_chunks[chunkIndex].locations[locationIndex].id = id;

	return id;
}

uint8_t* ChunkPool::get(uint32_t id){
	// Resolve id
	assert(id < _idCount);

	uint64_t pair = _ids[id];

	uint32_t chunkIndex = BitHelper::front(pair);
	uint32_t locationIndex = BitHelper::back(pair);

	assert(chunkIndex < _chunkCount);
	assert(locationIndex < _chunks[chunkIndex].locationCount);

	// Return byte pointer
	return _locationPointer(chunkIndex, locationIndex);
}

void ChunkPool::erase(uint32_t id){
	// Resolve id
	assert(id < _idCount);

	uint64_t pair = _ids[id];

	uint32_t chunkIndex = BitHelper::front(pair);
	uint32_t locationIndex = BitHelper::back(pair);

	assert(chunkIndex < _chunkCount);
	assert(locationIndex < _chunks[chunkIndex].locationCount);

	Chunk& chunk = _chunks[chunkIndex];
	Location& location = chunk.locations[locationIndex];

	// Update locations to the right of erased
	Location* iterLoc = &location;

	size_t size = location.endSize - location.startSize;

	while (BitHelper::getBit(iterLoc->flags, Location::RightExists)){
		iterLoc = &chunk.locations[iterLoc->rightLocation];

		iterLoc->startSize -= size;
		iterLoc->endSize -= size;
	}

	// Copy memory over to fill the gap
	if (locationIndex != chunk.lastLocation){
		uint8_t* pointer = _locationPointer(chunkIndex, locationIndex);

		std::memcpy(pointer, pointer + size, _chunkSize - location.endSize);
	}

	// Erase location and push id onto free stack
	_eraseLocation(chunkIndex, locationIndex);

	_freeIds.push(id);	
}

ChunkPool::Iterator ChunkPool::begin(){
	for (unsigned int i = 0; i < _chunkCount; i++){
		if (_chunks[i].locationCount)
			return Iterator(*this, _chunks[i].locations[_chunks[i].firstLocation].id);
	}

	return Iterator(*this);
}

unsigned int ChunkPool::count() const{
	unsigned int count = 0;

	for (unsigned int i = 0; i < _chunkCount; i++){
		count += _chunks[i].locationCount;
	}

	return count;
}

void ChunkPool::print() const{
	std::cout << "\n-------------\n";

	for (unsigned int x = 0; x < _chunkCount; x++){
		Chunk& chunk = _chunks[x];

		std::cout << "Chunk:\t" << x << "\n";
		//std::cout << "Count:\t" << chunk.locationCount << "\n";

		if (chunk.locationCount){
			std::cout << "First:\t" << chunk.firstLocation << "\n";
			std::cout << "Last:\t" << chunk.lastLocation << "\n";
		}

		std::cout << "\n";

		for (unsigned int y = 0; y < chunk.locationCount; y++){
			Location& location = chunk.locations[y];

			//std::cout << " Index:\t" << location.index << "\n";
			std::cout << " Id:\t" << location.id << "\n";
			std::cout << " Start:\t" << location.startSize << "\n";
			std::cout << " End:\t" << location.endSize << "\n";
			std::cout << " Size:\t" << location.endSize - location.startSize << "\n";

			if (BitHelper::getBit(location.flags, Location::LeftExists))
				std::cout << " Left:\t" << location.leftLocation << "\n";

			if (BitHelper::getBit(location.flags, Location::RightExists))
				std::cout << " Right:\t" << location.rightLocation << "\n";

			std::cout << "\n";
		}
	}
}
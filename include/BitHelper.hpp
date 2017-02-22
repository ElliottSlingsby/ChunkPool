#pragma once

#include <cstdint>

namespace BitHelper{
	template <typename T>
	inline T setBit(T bits, unsigned int i, bool value);

	template <typename T>
	inline bool getBit(T bits, unsigned int i);

	inline uint32_t front(uint64_t i);

	inline uint32_t back(uint64_t i);

	inline uint64_t combine(uint32_t front, uint32_t back);
}

template<typename T>
T BitHelper::setBit(T bits, unsigned int i, bool value){
	if (value)
		return bits | (1 << i);

	return bits & ~(1 << i);
}

template<typename T>
bool BitHelper::getBit(T bits, unsigned int i){
	return ((1 << i) & bits) != 0;
}

uint32_t BitHelper::back(uint64_t i){
	return (uint32_t)i;
}

uint32_t BitHelper::front(uint64_t i){
	return (uint32_t)(i >> 32);
}

uint64_t BitHelper::combine(uint32_t front, uint32_t back){
	return back + ((uint64_t)front << 32);
}
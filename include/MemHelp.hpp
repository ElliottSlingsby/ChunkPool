#pragma once

#define _CRT_SECURE_NO_DEPRECATE 

#include <cstdint>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <cstdio>

namespace MemHelp{
	struct Info{
		uint64_t start;
		uint64_t size;

		inline Info(uint64_t start = 0, uint64_t size = 0) :
			start(start),
			size(size){
		}

		template <typename T>
		inline Info(const T& other){
			start = other.start;
			size = other.size;
		}

		inline Info(const int& other){
			start = 0;
			size = 0;
		}

		inline operator bool() const{
			return size != 0;
		}

		template <typename T>
		inline void operator=(const T& other){
			start = other.start;
			size = other.size;
		}

		inline void operator=(const int& other){
			start = 0;
			size = 0;
		}

		template <typename T>
		inline bool operator==(const T& other) const{
			return start == other.start && size == other.size;
		}

		inline bool operator==(const int& other) const{
			return size != 0;
		}

		template <typename T>
		inline bool adjacent(const T& other) const{
			return start == other.end() || other.start == end();
		}

		inline uint64_t end() const{
			return start + size;
		}

		template <typename T>
		inline T combine(const T& other) const{
			if (other.start > start)
				return T(start, size + other.size);

			return T(other.start, size + other.size);
		}

		template <typename T>
		inline T subtract(const T& other) const{
			return T(start + other.size, size - other.size);
		}
	};

	struct Location : public Info{
		inline Location(uint64_t start = 0, uint64_t size = 0) :
			Info(start, size){
		}

		template <typename T>
		inline Location(const T& other) :
			Info(other){
		}

		inline bool operator<(const Location& other) const{
			return start < other.start;
		}
	};

	struct Size : public Info{
		inline Size(uint64_t start = 0, uint64_t size = 0) :
			Info(start, size){
		}

		template <typename T>
		inline Size(const T& other) :
			Info(other){
		}

		inline bool operator<(const Size& other) const{
			return size < other.size;
		}
	};

	class BaseType{
	protected:
		static uint64_t _typeCounter;

	public:
		virtual ~BaseType(){}
	};

	uint64_t BaseType::_typeCounter = 0;

	template <class T>
	class Type : public BaseType{
	public:
		static inline uint64_t type(){
			static uint64_t type = _typeCounter++;
			return type;
		};
	};

	inline uint8_t* allocate(uint64_t size, void* data){
		void* newData = nullptr;

		if (!data)
			newData = std::malloc(size);
		else
			newData = std::realloc(data, size);

		assert(newData);

		return (uint8_t*)newData;
	}

	inline uint8_t getHex(void* ptr, uint64_t pos){
		uint8_t val = *((uint8_t*)ptr + pos / 2);

		if (pos % 2)
			val >>= 4;

		return val & 0xF;
	}

	inline void setHex(void* ptr, uint64_t pos, uint8_t val){
		val &= 0xF;

		if (pos){
			if (pos % 2)
				val <<= 4;

			pos = pos / 2;
		}

		uint8_t* npos = (uint8_t*)ptr + pos;

		std::memset(npos, val | *npos, 1);
	}
}
#pragma once

#include <cstdlib>

// Based on malloc for types with no constr/destr
// Really only for primitive types or structs of primitive types
// (avoids using std's list based stack)

template <typename T>
class FlatStack{
	T* _values = nullptr;
	unsigned int _valueCount = 0;

	unsigned int _freed = 0;

public:
	inline ~FlatStack();

	inline T top() const;

	inline void push(const T& value);
	inline void pop();

	inline bool empty() const;
};

template <typename T>
FlatStack<T>::~FlatStack(){
	if (_values)
		std::free(_values);
}

template <typename T>
T FlatStack<T>::top() const{
	return _values[_valueCount - 1];
}

template <typename T>
void FlatStack<T>::push(const T& value){
	if (_freed){
		_freed--;
	}
	else{
		if (!_values)
			_values = (T*)std::malloc(sizeof(T));
		else
			_values = (T*)std::realloc(_values, sizeof(T) * (_valueCount + 1));
	}

	_values[_valueCount] = value;
	_valueCount++;
}

template <typename T>
void FlatStack<T>::pop(){
	_freed++;
	_valueCount--;
}

template <typename T>
bool FlatStack<T>::empty() const{
	return _valueCount == 0;
}
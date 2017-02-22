#include "ChunkPool.hpp"

#include <iostream>
#include <ctime>
#include <random>

// Testing code, shouldn't really be here

struct Test{
	inline Test(int x = 0, int y = 0, int z = 0) : x(x), y(y), z(z){}

	inline void print() const{
		std::cout << x << ", " << y << ", " << z << "\n";
	}

	int x;
	int y;
	int z;

	//uint8_t filler[100]; // bytes
};

int main(int argc, char *argv[]){
	srand((unsigned int)time(nullptr));

	std::cout << sizeof(Test) << "\n";

	ChunkPool pool(32 * 1024); // 32 KB

	unsigned int count = 1000000;

	for (unsigned int i = 0; i < count; i++){
		pool.set(sizeof(Test));
	}

	std::cout << pool.count() << "\n";

	for (unsigned int i = 0; i < count; i++){
		if (!(rand() % 2)) // randomly remove half all elements
			pool.erase(i);
	}

	std::cout << pool.count() << "\n";

	return 0;
}
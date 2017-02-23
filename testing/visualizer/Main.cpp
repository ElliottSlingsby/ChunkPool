#include "ChunkPool.hpp"

#include <iostream>
#include <ctime>
#include <random>
#include <chrono>

// Testing code, shouldn't really be here

struct Test{
	inline Test(int x = 0, int y = 0, int z = 0) : x(x), y(y), z(z){}

	inline void print() const{
		std::cout << x << ", " << y << ", " << z << "\n";
	}

	int x;
	int y;
	int z;

	uint8_t filler[256]; // bytes
};

int main(int argc, char *argv[]){
	srand((unsigned int)time(nullptr));

	ChunkPool pool(32 * 1024); // 32 KB

	unsigned int count = 100000;

	for (unsigned int i = 0; i < count; i++){
		Test& test = *((Test*)pool.get(pool.set(sizeof(Test))));

		test = Test(rand(), rand(), rand());
	}

	std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
	std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();

	std::chrono::high_resolution_clock::duration dt = end - start;


	float max = 5.f;

	float seconds = max;
	unsigned int counter = 0;

	while (seconds > 0){
		dt = end - start;
		
		start = std::chrono::high_resolution_clock::now();
	
		auto iter = pool.begin();
	
		while (iter.valid()){
			*((Test*)iter.get()) = Test(rand(), rand(), rand());
	
			iter.next();
		}
	
		//std::cout << std::chrono::duration_cast<std::chrono::duration<float>>(dt).count() * 1000 << " ms\n";

		seconds -= std::chrono::duration_cast<std::chrono::duration<float>>(dt).count();
		counter++;

		end = std::chrono::high_resolution_clock::now();
	}

	std::cout << "FPS : " << counter / max << "\n";


	start = std::chrono::high_resolution_clock::now();

	// Randomly remove half all elements
	for (unsigned int i = 0; i < count; i++){
		if (!(rand() % 100))
			pool.erase(i);
	}

	end = std::chrono::high_resolution_clock::now();

	dt = end - start;

	std::cout << "Removing : " << std::chrono::duration_cast<std::chrono::duration<float>>(dt).count() * 1000 << " ms\n";

	std::getchar();

	return 0;
}
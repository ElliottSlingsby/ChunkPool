#include "ObjectPool.hpp"

#include <ctime>
#include <cstdlib>
#include <cstdio>

// Save load file (relative to exe)
#define FILE "test.dat"

// Max object size to create
#define MAX 0xFFFF
// Amount of random sizes to create
#define SIZES 32

// Object pool allocation chunk size
#define CHUNK 0xFFFFFFF
// Amount of objects to insert using random sizes
#define OBJECTS 0xFFFF

int main(int argc, char *argv[]){
	srand((unsigned int)time(nullptr));

	std::vector<uint64_t> sizes;
	
	for (uint64_t i = 0; i < SIZES; i++){
		sizes.push_back((rand() % MAX) + 1);
	}
	
	ObjectPool pool(CHUNK);


	// Check if dat file exists
	if (std::fopen(FILE, "r") != nullptr){
		printf("Loading '%s.dat'...", FILE);
		pool.load(FILE);
	}
	else{
		for (uint64_t i = 0; i < OBJECTS; i++){
			pool.insert(sizes[rand() % sizes.size()]);
			printf("Adding %d %d\n", i, pool.totalSize());
		}

		printf("Saving '%s.dat'...", FILE);
		pool.save(FILE);
	}

	uint64_t printCounter = 0;

	while (1){
		pool.set(rand() % OBJECTS, sizes[rand() % sizes.size()]);

		ObjectPool::Iterator i = pool.begin();

		while (i.valid()){
			//printf("%d\n", i.size());
			i.next();
		}

		float size = (float)(pool.usedSize() + pool.gapSize());
		float percent = (100.f / size) * (float)pool.usedSize();

		printf("Used %f percent - Size %ld b - Excess %ld b\n", percent, pool.totalSize(), pool.topSize());
	}

	return 0;
}
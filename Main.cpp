#include "ObjectPool.hpp"

#include <ctime>
#include <random>
#include <list>

#define CHUNK 0xFFFFFFF
#define BLOCKS 0xFFF

#define MAX 0xFFFFFFFF
#define SIZES 32

#define FILE "test.dat"

int main(int argc, char *argv[]){
	srand((unsigned int)time(nullptr));

	std::vector<uint64_t> sizes = {
		0xF,
		0xFF,
		0xFFF,
		0xFFFF,
		0x1FFFF
	};
	
	for (uint64_t i = 0; i < SIZES; i++){
		//sizes.push_back((rand() % MAX) + 1);
	}

	ObjectPool pool(CHUNK);

	///*
	for (uint64_t i = 0; i < BLOCKS; i++){
		pool.insert(sizes[rand() % sizes.size()]);
		printf("Adding %d %d\n", i, pool.totalSize());
	}

	pool.save(FILE);
	//*/

	/*
	pool.load(FILE);
	*/

	uint64_t printCounter = 0;

	while (1){
		pool.set(rand() % BLOCKS, sizes[rand() % sizes.size()]);

		ObjectPool::Iterator i = pool.begin();

		while (i.valid()){
			//printf("%d\n", i.size());
			i.next();
		}

		float size = (float)(pool.usedSize() + pool.gapSize());
		float percent = (100.f / size) * (float)pool.usedSize();

		printf("Used %f percent - Size %ld bytes\n", percent, pool.totalSize());
	}

	return 0;
}
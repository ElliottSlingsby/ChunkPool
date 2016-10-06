#include "ObjectPool.hpp"
#include "TypePool.hpp"

#include <ctime>
#include <random>
#include <list>

#define CHUNK 100000000
#define BLOCKS 1024 * 4
#define MAX 1024 * 64
#define SIZES 10000

#define PRINT 10000

int main(int argc, char *argv[]){
	srand((unsigned int)time(nullptr));

	std::vector<uint64_t> sizes = {
		//8,
		//16,
		//32,
		//64,
		//128,
		//256,
		//512,
		//1024,
	};
	
	for (uint64_t i = 0; i < SIZES; i++){
		sizes.push_back((rand() % MAX) + 1);
	}

	ObjectPool pool(CHUNK);

	for (uint64_t i = 0; i < BLOCKS; i++){
		pool.insert(sizes[rand() % sizes.size()]);
		printf("%d %d\n", i, pool.totalSize());
	}

	uint64_t printCounter = 0;

	while (1){
		pool.set(rand() % BLOCKS, sizes[rand() % sizes.size()]);

		//if (printCounter > PRINT){
			float size = (float)(pool.usedSize() + pool.gapSize());
			float percent = (100.f / size) * (float)pool.usedSize();

			printf("Used %f percent - Size %ld bytes\n", percent, pool.totalSize());
		//	printCounter = 0;
		//}
		//else{
		//	printCounter++;
		//}		
	}

	return 0;
}
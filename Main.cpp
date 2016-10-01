#include "ObjectPool.hpp"
#include "MemHelp.hpp"

#include <ctime>
#include <random>
#include <list>

#define CHUNK RAND_MAX * 4
#define LOOPS 1024
#define BLOCKS 1024

int main(int argc, char *argv[]){
	srand((unsigned int)time(nullptr));

	ObjectPool pool(CHUNK);
	std::list<uint64_t> ids;

	for (unsigned int n = 0; n < LOOPS; n++){
		for (unsigned int i = 0; i < BLOCKS; i++){
			ids.push_back(pool.insert(rand() + 1));
		}

		printf("Before - %d\n", pool.size());

		for (unsigned int i = 0; i < BLOCKS; i++){
			auto iter = ids.begin();
			std::advance(iter, rand() % ids.size());

			pool.remove(*iter);
			ids.remove(*iter);
		}

		pool.processRemoved();

		printf("After  - %d\n", pool.size());
	}
	
	return 0;
}
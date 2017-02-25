#include "TypePool.hpp"

#include <gtest\gtest.h>

struct Banana{
	unsigned int x;
	uint8_t size[8];
	unsigned int y;
};

struct Dog{
	unsigned int x;
	uint8_t size[16];
	unsigned int y;
};

struct Toronto{
	unsigned int x;
	uint8_t size[32];
	unsigned int y;
};

struct Wizard{
	unsigned int x;
	uint8_t size[64];
	unsigned int y;
};

struct Puzzle{
	unsigned int x;
	uint8_t size[128];
	unsigned int y;
};

TEST(TypePoolTest, Testing){ // TODO: make actual unit tests
	TypePool pool(32 * 1024); //32 KB

	uint32_t d = pool.insert<Banana, Dog, Puzzle, Toronto, Wizard>();
	uint32_t a = pool.insert<Banana, Dog, Puzzle, Toronto, Wizard>();
	uint32_t e = pool.insert<Banana, Puzzle, Wizard>();

	pool.execute([](const TypePool::Mask& mask, Banana* banana, Dog* dog, Puzzle* puzzle){
		for (unsigned int i = 0; i < mask.length<Banana>(); i++){
			//banana[i];
		}

		std::cout << "Found!\n";

		return;
	});
}
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

TEST(TypePoolTest, Testing){
	TypePool pool(32 * 1024); //32 KB

	uint32_t idA = pool.insert<Banana, Dog, Puzzle, Toronto, Wizard>(1, 1, 1, 1, 1);
	uint32_t idB = pool.insert<Banana, Dog, Puzzle, Wizard>(1, 1, 5, 1);
	uint32_t idC = pool.insert<Banana, Dog, Toronto, Puzzle>(1, 1, 2, 0);

	Puzzle* puzzle = pool.get<Puzzle>(idB);

	for (unsigned int i = 0; i < pool.length<Puzzle>(idB); i++){
		puzzle[i].x = i;
		puzzle[i].y = i;
	}

	pool.execute([](const TypePool::Mask& mask, Puzzle* puzzle, Dog* dog, Banana* banana){
		std::cout << "Found!\n";

		for (unsigned int i = 0; i < mask.length<Puzzle>(); i++){
			std::cout << puzzle[i].x << ", " << puzzle[i].y << "\n";
		}

		return;
	});
}
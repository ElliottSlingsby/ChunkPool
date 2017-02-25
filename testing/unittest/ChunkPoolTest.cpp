#include "ChunkPool.hpp"

#include <gtest\gtest.h>
#include <list>
#include <vector>
#include <tuple>

#define CHUNK 32 * 1024
#define BLOCKS 100000

class TestObject{
public:
	TestObject(int x = 0, int y = 0, int z = 0) : x(x), y(y), z(z){}
	
	int operator==(const TestObject& other){
		return (other.x == x) && (other.y == y) && (other.z == z);
	}

	int x;
	int y;
	int z;
};

TEST(ChunkPoolTest, Iteration){
	ChunkPool pool(CHUNK);

	std::vector<std::pair<uint32_t, TestObject>> added;
	added.resize(BLOCKS);

	for (unsigned int i = 0; i < BLOCKS; i++){
		unsigned int index = pool.insert(sizeof(TestObject));

		added[index] = { index, TestObject(rand(), index, rand()) }; // setting y to index for later

		(*(TestObject*)pool.get(index)) = added[index].second;
	}

	ChunkPool::Iterator iter = pool.begin();

	unsigned int counter = 100;

	while (counter){
		iter = pool.begin();

		while (iter.valid()){
			EXPECT_TRUE(added[((TestObject*)iter.get())->y].second == (*(TestObject*)iter.get()));
			iter.next();
		}

		counter--;
	}
}

TEST(ChunkPoolTest, InsertErase){
	ChunkPool pool(CHUNK);

	std::list<std::pair<uint32_t, TestObject>> added;

	for (unsigned int i = 0; i < BLOCKS; i++){
		added.push_back({ pool.insert(sizeof(TestObject)), TestObject(rand(), rand(), rand()) });
		(*(TestObject*)pool.get(added.rbegin()->first)) = added.rbegin()->second;
	}

	std::list<std::pair<uint32_t, TestObject>>::iterator iter = added.begin();

	while (iter != added.end()){
		if (!(rand() % 2)){
			pool.erase(iter->first);
			iter = added.erase(iter);
		}
		else{
			iter++;
		}
	}

	for (auto& pair : added){
		EXPECT_TRUE(pair.second == (*(TestObject*)pool.get(pair.first)));
	}
}
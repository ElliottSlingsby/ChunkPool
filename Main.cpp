#include "ObjectPool.hpp"
#include "MemHelp.hpp"

#include <ctime>
#include <random>
#include <list>

#define CHUNK 0xFFFF
#define LOOPS 1024
#define BLOCKS 1024

struct TestStruct{
	char string[256] = "";
	float number = 0.f;
	int integer = 0;
	bool boolean = false;
};

int main(int argc, char *argv[]){
	ObjectPool pool(CHUNK);


	const char testStr0[] = "Fusce ac turpis aliquet, tristique diam id, venenatis nisl.";
	const char testStr1[] = "Integer congue tortor euismod tristique elementum.";
	const char testStr2[] = "Ut eget nulla vulputate, vestibulum neque.";
	const char testStr3[] = "Sed fringilla.";
	const char testStr4[] = "Praesent gravida risus eu orci viverra, in eleifend magna aliquam.";
	const char testStr5[] = "Donec egestas diam et libero accumsan, eu dictum leo faucibus.";
	const char testStr6[] = "Pellentesque id massa eu nulla pretium.";
	const char testStr7[] = "Quisque suscipit ex ac.";

	uint64_t index0 = pool.insert(sizeof(testStr0));
	uint64_t index1 = pool.insert(sizeof(testStr1));
	uint64_t index2 = pool.insert(sizeof(testStr2));
	uint64_t index3 = pool.insert(sizeof(testStr3));
	uint64_t index4 = pool.insert(sizeof(testStr4));
	uint64_t index5 = pool.insert(sizeof(testStr5));
	uint64_t index6 = pool.insert(sizeof(testStr6));
	uint64_t index7 = pool.insert(sizeof(testStr7));

	std::memcpy(pool.get(index0), testStr0, sizeof(testStr0));
	std::memcpy(pool.get(index1), testStr1, sizeof(testStr1));
	std::memcpy(pool.get(index2), testStr2, sizeof(testStr2));
	std::memcpy(pool.get(index3), testStr3, sizeof(testStr3));
	std::memcpy(pool.get(index4), testStr4, sizeof(testStr4));
	std::memcpy(pool.get(index5), testStr5, sizeof(testStr5));
	std::memcpy(pool.get(index6), testStr6, sizeof(testStr6));
	std::memcpy(pool.get(index7), testStr7, sizeof(testStr7));
		

	//uint64_t testIndex = pool.insert(sizeof(testStr7));

	//pool.set(10, 0);


	ObjectPool::Iterator iter = pool.begin();

	while (iter.valid()){
		printf("%s\n", (char*)iter.get());

		iter.next();
	}


	srand((unsigned int)time(nullptr));

	std::list<uint64_t> ids;

	while (1){
	//for (unsigned int n = 0; n < LOOPS; n++){
		for (unsigned int i = 0; i < BLOCKS; i++){
			uint64_t r = rand() + 1;
			ids.push_back(pool.insert(r));
		}

		printf("%d\n", pool.bufferSize());

		for (unsigned int i = 0; i < BLOCKS; i++){
			auto iter = ids.begin();
			std::advance(iter, rand() % ids.size());

			pool.remove(*iter);
			ids.remove(*iter);
		}

		pool.freeRemoved();
	}
	
	return 0;
}
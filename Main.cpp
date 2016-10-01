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


	pool.remove(index0);
	pool.remove(index1);
	pool.remove(index3);
	pool.remove(index6);
	pool.remove(index5);

	pool.processRemoved();

	index0 = pool.insert(sizeof(testStr0));
	index1 = pool.insert(sizeof(testStr1));
	index3 = pool.insert(sizeof(testStr3));
	index5 = pool.insert(sizeof(testStr5));
	index6 = pool.insert(sizeof(testStr6));

	//std::memcpy(pool.get(index0), testStr0, sizeof(testStr0));
	//std::memcpy(pool.get(index1), testStr1, sizeof(testStr1));
	//std::memcpy(pool.get(index3), testStr3, sizeof(testStr3));
	//std::memcpy(pool.get(index5), testStr5, sizeof(testStr5));
	//std::memcpy(pool.get(index6), testStr6, sizeof(testStr6));

	
	char* content0 = (char*)pool.get(index0);
	char* content1 = (char*)pool.get(index1);
	char* content2 = (char*)pool.get(index2);
	char* content3 = (char*)pool.get(index3);
	char* content4 = (char*)pool.get(index4);
	char* content5 = (char*)pool.get(index5);
	char* content6 = (char*)pool.get(index6);
	char* content7 = (char*)pool.get(index7);

	if (content0)
		printf("Equal = %d %s\n", std::strcmp(testStr0, content0) == 0, content0);
	if (content1)
		printf("Equal = %d %s\n", std::strcmp(testStr1, content1) == 0, content1);
	if (content2)
		printf("Equal = %d %s\n", std::strcmp(testStr2, content2) == 0, content2);
	if (content3)
		printf("Equal = %d %s\n", std::strcmp(testStr3, content3) == 0, content3);
	if (content4)
		printf("Equal = %d %s\n", std::strcmp(testStr4, content4) == 0, content4);
	if (content5)
		printf("Equal = %d %s\n", std::strcmp(testStr5, content5) == 0, content5);
	if (content6)
		printf("Equal = %d %s\n", std::strcmp(testStr6, content6) == 0, content6);
	if (content7)
		printf("Equal = %d %s\n", std::strcmp(testStr7, content7) == 0, content7);

	
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
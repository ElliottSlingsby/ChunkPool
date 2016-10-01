#include "ObjectPool.hpp"
#include "MemHelp.hpp"

#include <ctime>
#include <random>
#include <list>

int main(int argc, char *argv[]){
	srand((unsigned int)time(nullptr));

	ObjectPool pool(RAND_MAX * 4);
	std::list<uint64_t> ids;

	for (unsigned int n = 0; n < 8; n++){
		for (unsigned int i = 0; i < 1024; i++){
			ids.push_back(pool.insert(rand() + 1));
		}

		printf("Before - %d\n", pool._bufferSize);

		for (unsigned int i = 0; i < 1024; i++){
			auto iter = ids.begin();
			std::advance(iter, rand() % ids.size());

			pool.remove(*iter);
			ids.remove(*iter);
		}

		pool.processRemoved();

		printf("After - %d\n", pool._bufferSize);
	}
	
	return 0;

	/*PoolPointer pointer = pool.newPointer<Transform, Model, Script, Script, Script>();

	for (unsigned int i = i; i < pointer.has<Script>(); i++){
		pointer.write<Script>(i)->tableId = i + 1;
	}

	pointer.invalidate();


	PoolPointer i = pool.begin();

	while (i){
		if (i.has<Transform, Model>()){
			i.write<Transform>()->x = 1.f;
			i.write<Transform>()->y = 2.f;
			i.write<Transform>()->z = 3.f;

			printf(i.read<Model>()->source);
		}

		i.erase();
		i++;
	}

	pool.processErased();

	return 0;*/
}
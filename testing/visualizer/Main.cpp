#include "VisualizerPool.hpp"

#include <ctime>
#include <cstdlib>
#include <cstdio>

int main(int argc, char *argv[]){
	srand((unsigned int)time(nullptr));

	VisualizerPool pool(512);
		
	while (!pool.shouldClose()){
		pool.set(rand() % 16, rand() % 64);

		pool.draw();
	}

	return 0;
}
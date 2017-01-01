#include "ThreadSequencer.hpp"

#include <cstdio>

int main(int argc, char *argv[]){
	ThreadSequencer sequencer(64, 8);

	sequencer.start();

	std::atomic<int> x = 0;

	sequencer.propagate(1024, [&](void* data){
		x++;
	});

	sequencer.wait();

	sequencer.propagate(1024, [](void* data){

	});

	sequencer.wait();

	return 0;
}
#pragma once

#include "ObjectPool.hpp"

#include <vector>
#include <thread>
#include <atomic>
#include <functional>

class ThreadSequencer{
	struct ThreadInfo{
		ObjectPool* pool = nullptr;

		std::atomic<unsigned int> counter = 0;
		std::atomic<unsigned int> waiting = 0;
	};

	unsigned int _maxThreads = 0;

	std::atomic<unsigned int> _running = false;

	std::vector<std::thread> _threads;
	std::vector<ThreadInfo*> _info;

	std::function<void(void*)>* _job = nullptr;

	inline static unsigned int _thread(std::atomic<unsigned int>& running, ThreadInfo& info, std::function<void(void*)>* job);

public:
	// Create all thread info and memory space
	inline ThreadSequencer(unsigned int chunkSize, unsigned int maxThreads = 0);

	// Abort and join all threads, and free info and memory space
	inline ~ThreadSequencer();

	// Create threads and set running
	inline void start();

	// Propagate amount of jobs to threads
	inline void propagate(unsigned int amount, std::function<void(void*)> job);

	// Wait for propogated jobs to finish
	inline void wait();

	// Wait for threads to finish jobs, or force stop
	inline void stop(bool abort = true);

	// Abort and clear all info and memory space
	inline void clear();
};

inline unsigned int ThreadSequencer::_thread(std::atomic<unsigned int>& running, ThreadInfo& info, std::function<void(void*)>* job){
	//printf("S %d %d %d %d %d\n", running, info.counter, info.waiting, info.events.size(), info.pool->totalSize());

	while (running){
		if (!info.counter || info.waiting)
			continue;

		for (unsigned int i = 0; i < 1024; i++){
			std::sqrt(rand() * rand() * rand());
		}

		(*job)(nullptr); // objects from info.pool will be given as parameters to job

		info.counter--;
	}
	
	return 0;
}

inline ThreadSequencer::ThreadSequencer(unsigned int chunkSize, unsigned int maxThreads){
	if (maxThreads)
		_maxThreads = maxThreads;
	else
		_maxThreads = std::thread::hardware_concurrency();

	_threads.resize(_maxThreads);
	_info.resize(_maxThreads);

	for (unsigned int i = 0; i < _maxThreads; i++){
		_info[i] = new ThreadInfo();
		_info[i]->pool = new ObjectPool(chunkSize);
		_info[i]->pool->reserve();
	}

	_job = new std::function<void(void*)>();
}

inline ThreadSequencer::~ThreadSequencer(){
	stop();

	for (unsigned int i = 0; i < _threads.size(); i++){
		delete _info[i]->pool;
		delete _info[i];
	}
}

inline void ThreadSequencer::start(){
	if (_running)
		return;

	_running = 1;

	for (unsigned int i = 0; i < _maxThreads; i++){
		_threads[i] = std::thread(_thread, std::ref(_running), std::ref(*_info[i]), _job);
	}
}

inline void ThreadSequencer::propagate(unsigned int amount, std::function<void(void*)> job){
	unsigned int sub = (unsigned int)round((double)amount / (double)_maxThreads);

	*_job = job;

	if (!sub)
		sub = amount;

	unsigned test = 0;

	for (ThreadInfo* info : _info){
		if (sub >= amount){
			info->counter += amount;
			test += amount;
			break;
		}

		test += sub;
		info->counter += sub;

		amount -= sub;
	}
}

inline void ThreadSequencer::wait(){
	if (!_running)
		return;

	unsigned int jobs;

	do{
		jobs = 0;

		for (ThreadInfo* info : _info){
			jobs += info->counter;
		}
	} while (jobs);
}

inline void ThreadSequencer::stop(bool abort){
	if (!_running)
		return;
	
	if (!abort)
		wait();

	_running = 0;

	for (unsigned int i = 0; i < _threads.size(); i++){
		_threads[i].join();
	}
}

inline void ThreadSequencer::clear(){
	stop(true);

	for (ThreadInfo* info : _info){
		info->pool->clear();
		info->pool->reserve();

		info->counter = 0;
		info->waiting = 0;
	}
}

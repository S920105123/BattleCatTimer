#ifndef BCT_SAFE_QUEUE
#define BCT_SAFE_QUEUE

#include "header.h"

template<class T>
class SafeQueue 
{
public:
	SafeQueue(){}

	bool wait_for_date(T& x) {
		std::unique_lock<std::mutex> lock(mut);
		cv.wait(lock, [&](){ return !Q.empty(); });

		x = Q.front();
		Q.pop();
		return true;
	}

	void push(const T& x) {
		std::unique_lock<std::mutex> lock(mut);
		Q.push(x);

		lock.unlock();
		cv.notify_one();
	}

	bool try_get_and_pop(T& x) {
		std::lock_guard<std::mutex> lock(mut);
		if(Q.empty()) return false;
		x = Q.front();
		Q.pop();
		return true;
	}

	void clear() {
		std::lock_guard<std::mutex> lock(mut);
		while(!Q.empty()) Q.pop();
	}

	bool empty() {
		std::lock_guard<std::mutex> lock(mut);
		return Q.empty();
	}

	int size() {
		std::lock_guard<std::mutex> lock(mut);
		return Q.size();
	}

private:
	queue<T> Q;
	std::mutex mut;
	std::condition_variable cv;
};

#endif

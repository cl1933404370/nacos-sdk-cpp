#ifndef __BLOCKING_Q_H_
#define __BLOCKING_Q_H_

#include <deque>
#include "src/thread/Mutex.h"
/*
* BlockingQueue.h
* Thanks to Shuo, Chen's muduo: https://github.com/chenshuo/muduo/blob/master/muduo/base/BlockingQueue.h
*/

namespace nacos{
template<typename T>
class BlockingQueue
{
private:
	Mutex _mutex;
	Condition _notEmpty;
	Condition _notFull;
	std::deque<T> _queue;
	size_t _maxSize;
	std::atomic_bool _full;
	std::atomic_bool _empty;
public:

    bool full() {
        LockGuard lockguard(&_mutex);
        return _full;
    };
    bool empty() {
        LockGuard lockguard(&_mutex);
        return _empty;
    };

	BlockingQueue() : _mutex(), _notEmpty(&_mutex), _notFull(&_mutex), _maxSize(64), _full(false), _empty(true) {};
	BlockingQueue(size_t queueSize) : _mutex(), _notEmpty(&_mutex), _notFull(&_mutex), _maxSize(queueSize), _full(false), _empty(true) {};
	void enqueue(const T &data)
	{
		LockGuard lockguard(&_mutex);
		_full = true;
		_notFull.wait([&]{return _queue.size() != _maxSize;});
		_full = false;
		_queue.push_back(data);
		_full = _queue.size() == _maxSize;
	    _empty = false;
		_notEmpty.notify();
	}

	T dequeue()
	{
		LockGuard lockguard(&_mutex);
		_empty = true;
		_notEmpty.wait([&]{return !_queue.empty();});
		_empty = false;
		T front = _queue.front();
		_queue.pop_front();
		_empty = _queue.empty();
		_full = false;
		_notFull.notify();
		return front;
	}
};
}//namespace nacos

#endif
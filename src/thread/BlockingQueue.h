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
	volatile bool _full;
	volatile bool _empty;
public:

    bool full() {
        LockGuard lockguard(_mutex);
		this->_full = _queue.size() == _maxSize ? true : false;
        return this->_full;
    };
    bool empty() {
        LockGuard lockguard(_mutex);
		_empty = _queue.empty();
        return _empty;
    };

	BlockingQueue() : _mutex(), _notEmpty(_mutex), _notFull(_mutex), _maxSize(64), _full(false), _empty(true) {};
	BlockingQueue(size_t queueSize) : _mutex(), _notEmpty(_mutex), _notFull(_mutex), _maxSize(queueSize), _full(false), _empty(true) {};
	void enqueue(const T &data)
	{
		full();
		empty();
		{
			LockGuard lockguard(_mutex);
			_notFull.wait([this] {return _queue.size() != _maxSize;});
			_queue.push_back(data);
		}
		full();
		empty();
		_notEmpty.notify();
	}

	T dequeue()
	{
		
		full();
		empty();
		T front{};
		{
			LockGuard lockguard(_mutex);
			_notEmpty.wait([this] {
				return !_queue.empty(); });
			front = _queue.front();
			_queue.pop_front();
		}
		full();
		empty();
		_notFull.notify();
		return front;
	}
};
}//namespace nacos

#endif
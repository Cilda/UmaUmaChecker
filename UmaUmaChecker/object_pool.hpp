#pragma once

#include <list>
#include <unordered_set>
#include <condition_variable>
#include <mutex>

template<class T>
class object_pool
{
public:
	object_pool()
	{
	}

	virtual ~object_pool()
	{
		pools.clear();
		data.clear();
	}

	void join_manage_resource(T* obj)
	{
		data.emplace_back(obj);
		pools.push_back(obj);
	}

	T* get()
	{
		std::unique_lock lock(mutex);

		cond.wait(lock, [this] { return !pools.empty(); });
		T* ret = pools.back();
		pools.pop_back();
		return ret;
	}

	void release(T* ptr)
	{
		std::unique_lock lock(mutex);

		pools.push_back(ptr);
		cond.notify_all();
	}

private:
	size_t size;
	std::mutex mutex;
	std::condition_variable cond;
	std::list<T*> pools;
	std::list<std::unique_ptr<T>> data;
};


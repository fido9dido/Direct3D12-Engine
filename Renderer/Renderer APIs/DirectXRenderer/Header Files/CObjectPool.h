#ifndef COBJECTPOOL_H
#define COBJECTPOOL_H

#include <forward_list>
#include <functional>
#include <mutex>

template<typename T>
class CObjectPool
{
public:
private:
	mutable std::mutex Mutex;
	std::forward_list<std::shared_ptr<T>> IdlePool;
public:
	CObjectPool(){}
	std::shared_ptr<T> GetOrCreateObject()
	{
		std::lock_guard<std::mutex>(Mutex);
		if (Pool.empty())
		{
			return CreateObject();
		}

		std::shared_ptr<T>& result = IdlePool.front();
		IdlePool.pop_front();
		return result;
	}

	void ReturnToPool(std::shared_ptr<T>& object)
	{
		std::lock_guard<std::mutex>(Mutex);
		IdlePool.emplace_back(object);
	}

	void ForEeach(std::function<void(std::shared_ptr<T>&)> func)
	{
		std::lock_guard<std::mutex>(Mutex);
		for (std::shared_ptr<T>& resource: IdlePool)
		{
			func(resource);
		}
	}

	void ForEeach(std::function<void(const std::shared_ptr<T>&)> func)
	{
		std::lock_guard<std::mutex>(Mutex);
		for (const std::shared_ptr<T>& resource : IdlePool)
		{
			func(resource);
		}
	}
private:
	std::shared_ptr<T>& CreateObject()
	{
		return std::make_shared<T>();
	}
};

#endif
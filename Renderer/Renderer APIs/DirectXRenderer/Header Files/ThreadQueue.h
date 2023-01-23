#ifndef CTHREADQUEUE_H
#define CTHREADQUEUE_H

#include <memory>
#include <mutex>
#include <utility>
#include <queue>

template<typename T>
class CThreadQueue
{
public:
	mutable std::mutex Mutex;
private:
	std::queue<T> Data;
public:
	CThreadQueue(){}
	~CThreadQueue(){}

	CThreadQueue(const CThreadQueue& queue) noexcept(std::is_nothrow_default_constructible_v<T>)
	{
		std::lock_guard<std::mutex> lock(Mutex);
		Data = queue.Data;
	}
	CThreadQueue(const CThreadQueue&& queue) noexcept(std::is_nothrow_default_constructible_v<T>)
	{
		std::lock_guard<std::mutex> lock(Mutex);
		Data.Get() = std::move(queue);
	}


public:
	std::shared_ptr<T> TryPop()
	{
		std::shared_ptr<T> head = Pop();
		return head.get() ? head : std::shared_ptr<T>();
	}

	bool TryPop(T& value)
	{
		return Pop(value);
	}

	void Push(T& value)
	{
		std::lock_guard<std::mutex> lock(Mutex);
		Data.push(std::move(value));
	}
	void Emplace(T& value)
	{
		std::lock_guard<std::mutex> lock(Mutex);
		Data.emplace(std::move(value));
	}

	void Move(std::queue<T>& queue)
	{
		std::lock_guard<std::mutex> lock(Mutex);
		Data = std::move(queue);
	}

	void Size() const
	{
		std::lock_guard<std::mutex> lock(Mutex);
		return Data.size();
	}

	bool IsEmpty() const
	{
		std::lock_guard<std::mutex> lock(Mutex);
		return Data.empty();
	}
	void Clear()
	{
		std::lock_guard<std::mutex> lock(Mutex);
		std::queue<T> empty;
		std::swap(q, empty);
	}
	std::queue<T>& Get()
	{
		std::lock_guard<std::mutex> lock(Mutex);
		return Data;
	}
private:
	std::shared_ptr<T> Pop() 
	{
		std::lock_guard<std::mutex> lock(Mutex);
		if (Data.empty()) { return nullptr; };
		std::shared_ptr<T> front = std::make_shared(Data.front());
		Data.pop();
		return front;
	}
	bool Pop(T& value)
	{
		std::lock_guard<std::mutex> lock(Mutex);

		if (Data.empty()) { return false; };

		value = std::move(Data.front());
		Data.pop();
		return true;
		
	}
	
private:
	CThreadQueue& operator =(const CThreadQueue& queue) = delete;
};

#endif
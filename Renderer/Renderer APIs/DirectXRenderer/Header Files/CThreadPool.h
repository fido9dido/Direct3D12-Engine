#ifndef CTHREADPOOL_H
#define CTHREADPOOL_H

#include "ThreadQueue.h"
#include <functional>
#include <atomic>
#include <CThreads.h>
#include <CThreadTask.h>
#include <future>

class CThreadPool
{
public:
private:
	std::atomic_bool bDone;
	CThreadQueue<CThreadTask> WorkQueue;
	CThreads Threads;

public:
	CThreadPool();
	~CThreadPool();

	template<typename Task,typename ...Args>
	std::future<typename std::invoke_result_t<Task, Args...>> Submit(Task task,Args... args)
	{
		std::packaged_task<std::invoke_result_t<Task, Args...>()> newTask([f = std::move(task), largs = std::make_tuple(std::forward<Args>(args)...)]() mutable {
			return std::apply(std::move(f), std::move(largs));
			});
		std::future<std::invoke_result_t<Task, Args...>> result(newTask.get_future());
		WorkQueue.Emplace(CThreadTask(std::move(newTask)));

		return result;

	}

private:
	void WorkerThread();
};

#endif
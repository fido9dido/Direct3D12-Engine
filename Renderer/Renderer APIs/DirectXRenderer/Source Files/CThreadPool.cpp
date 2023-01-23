#include "CThreadPool.h"

CThreadPool::CThreadPool() : bDone(false)
{
	int threadCount = static_cast<int>(std::thread::hardware_concurrency());
	try
	{
		for (int i = 0; i < threadCount; ++i)
		{
			Threads.AddThread(std::thread(&CThreadPool::WorkerThread, this));
		}
	}
	catch (...)
	{
		bDone = true;
		throw 0;
	}
}

CThreadPool::~CThreadPool()
{
	bDone = true;
}

void CThreadPool::WorkerThread()
{
	while (!bDone)
	{
		CThreadTask task;
		
		if (WorkQueue.TryPop(task))
		{
			task();
			continue;
		}
		std::this_thread::yield();
	}
}

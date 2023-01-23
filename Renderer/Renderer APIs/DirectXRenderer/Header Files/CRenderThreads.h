#ifndef CRENDERTHREAD_H
#define CRENDERTHREAD_H

#include <CThreads.h>
#include <d3d12.h>

// These threads run indefinitely and put to sleep until notified 
// Currently 1 thread per window(Total of 4 windows)
// am mixing win32 threads with std which is not recommended https ://stackoverflow.com/questions/26056695/is-there-a-c-equivalent-of-waitforsingleobject
//but they are working fine for this case ;P 
class CRenderThreads
{

private:

	CThreads Threads;
	std::atomic_bool& bDone;
	// Handles to put threads to wake/sleep threads
	std::vector<HANDLE> WorkerBegin;
	std::vector<HANDLE> WorkerFinished;

public:
	CRenderThreads(std::atomic_bool& ref):bDone(ref) { }
	~CRenderThreads()
	{ 
		bDone = true;
		BeginRender();
	}
	template<typename Task, typename ...Args>
	void Submit(Task task, Args... args)
	{
		WorkerBegin.push_back(CreateEvent(
			NULL,
			FALSE,
			FALSE,
			NULL));
		WorkerFinished.push_back(CreateEvent(
			NULL,
			FALSE,
			FALSE,
			NULL));

		Threads.AddThread(std::thread(std::move(task), std::move(args)...));
		
	}

	void BeginRender()
	{
		for(HANDLE& wakeThread: WorkerBegin)
		{
			SetEvent(wakeThread);
		}
	}

	std::vector<HANDLE>& GetWorkerFinished()
	{
		return WorkerFinished;
	}

	HANDLE& GetWorkerFinished(int threadID)
	{
		return WorkerFinished[threadID];
	}

	HANDLE GetWorkerBeginHandle(int threadID)
	{
		return WorkerBegin[threadID];
	}
};

#endif // !CRENDERTHREAD
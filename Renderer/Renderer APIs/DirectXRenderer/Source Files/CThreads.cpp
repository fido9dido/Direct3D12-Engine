#include "CThreads.h"

CThreads::~CThreads()
{
	size_t count = ThreadList.size();
	for (size_t i=0;i< count;++i)
	{
		std::thread& thread = ThreadList[i];
		if (thread.joinable()) 
		{
			//std::this_thread::yield();
			thread.join();
		}
	}
}

void CThreads::AddThread(std::thread thread)
{
	ThreadList.push_back(std::move(thread));
}

void CThreads::Join()
{
	for (std::thread& thread : ThreadList)
	{
		if (thread.joinable())
		{
			thread.join();
		}
	}
}

#ifndef CTHREADS_H
#define CTHREADS_H

#include <vector>
#include <thread>

class CThreads
{
	std::vector<std::thread> ThreadList;
public:
	CThreads() = default;
	~CThreads();

	void AddThread(std::thread thread);
	std::thread::id GetID(int index) { return ThreadList[index].get_id(); }
	void Join();
private:
};
#endif

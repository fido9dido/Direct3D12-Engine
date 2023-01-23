#ifndef CTHREADTASK_H
#define CTHREADTASK_H

#include "Utility.h"

struct ITaskBase
{
	virtual ~ITaskBase(){}
	virtual void Call() = 0;
};
template<typename Func>
struct CTask : public ITaskBase
{
	Func Task;
public:
	CTask(Func&& task) : Task(std::move(task)) {}
	~CTask() {}

	void Call()
	{
		Task();
	}
};

//TODO ensure CTask type = ThreadTask T
class CThreadTask
{
public:
private:
	std::unique_ptr<ITaskBase> Task;
public:
	CThreadTask() = default;
	template<typename Func>
	CThreadTask(Func&& task)
		: Task(::make_unique<CTask<Func>>(std::move(task)))
	{}

	CThreadTask(CThreadTask&& other) noexcept
		: Task(std::move(other.Task))
	{}

	CThreadTask(CThreadTask& other) = delete;

	CThreadTask& operator= (CThreadTask&& other) noexcept
	{
		Task = std::move(other.Task);
		return *this;
	}
	CThreadTask operator= (const CThreadTask& other) = delete;
	CThreadTask operator= (CThreadTask& other) = delete;
	void operator()() { Task->Call(); }

private:
	
};

#endif

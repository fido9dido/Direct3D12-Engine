#ifndef CEVENT_H
#define CEVENT_H

#include <functional>
#include <vector>
#include <string>
#include "MemoryUtil.h"

/// <summary>
/// Class for non void observers
/// </summary>
/// <typeparam name="Return"> the return type of the registered functions </typeparam>
/// <typeparam name="...Args"></typeparam>
template <typename Return, typename... Args>
class CEventReturn
{
	using CObserver = std::function<Return(Args...)>;

	std::vector<CObserver> Observers;
public:

	CEventReturn() = default;

	void Register(CObserver callback)
	{
		Observers.emplace_back(callback);
	}

	std::vector<Return> Notify(const Args&... args)
	{
		std::vector<Return> result;
		for (const CObserver& callback : Observers)
		{
			result.pushback(callback(args...));
		}
		return result;
	}
};

template <typename... Args>
class CEvent
{
	using CObserver = std::function<void(Args...)>;

	std::vector<CObserver> Observers;
public:

	CEvent() = default;

	void Register(CObserver callback)
	{
		Observers.emplace_back(callback);
	}

	void Notify(const Args&... args) const
	{
		for (const CObserver& callback : Observers)
		{
			callback(args...);
		}
	}
};
#endif
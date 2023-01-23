#ifndef DXIMOUSEEVENT_H
#define DXIMOUSEEVENT_H

#include <Windows.h>

class DXIMouseEvent
{
public:
	enum class DEVICESTATUS : int
	{
		LReleased,
		RReleased,
		MReleased,
		LPressed,
		MPressed,
		RPressed,
		WheelUp,
		WheelDown,
		Move,
		RawMove,
		Invalid

	};
	enum class MOUSEBUTTONS : int
	{
		LeftB,
		RightB,
		MiddleB,
		Count
	};

	DXIMouseEvent();
	DXIMouseEvent(const DEVICESTATUS& deviceStatus, const POINT& position);
	const DEVICESTATUS& GetMouseStatus() const;
	const POINT& GetPosition() const;
	LONG GetXPosition() const;
	LONG GetYPosition() const;;
	bool IsValid() const;

private:
	DEVICESTATUS m_DeviceStatus;
	POINT m_Position;

};


#endif
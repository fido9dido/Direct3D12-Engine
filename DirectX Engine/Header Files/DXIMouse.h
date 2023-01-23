#ifndef DXIMOUSE_H
#define DXIMOUSE_H

#include "DXIMouseEvent.h"
#include <queue>
#include <array>

enum class MOUSEBUTTONS : int
{
	LeftB,
	RightB,
	MiddleB,
	Count
};

class DXIMouse //: public IInputDevice
{
public:

	
	DXIMouse();

	void ResetPosition();
	/// <summary>
	///  mouse event where we  want to track whether buttons are pressed
	/// </summary>
	/// <param name="e">mouse Press / Released Event</param>
	/// <param name="p"> Mouse Position</param>
	/// <param name="val"> bool for is Button Pressed</param>
	/// <param name="i"> index for which button is pressed, </param>
	void OnMainButtons(const DXIMouseEvent::DEVICESTATUS& e, const POINT& p, bool val, const DXIMouseEvent::MOUSEBUTTONS& i);
	/// <summary>
	///  mouse event where we dont  want to track whether buttons are pressed
	/// </summary>
	/// <param name="e">mouse Press / Released Event</param>
	/// <param name="p"> Mouse Position</param>
	void SetMouse(const DXIMouseEvent::DEVICESTATUS& e, const POINT& p);
	void SetRawMouse(const DXIMouseEvent::DEVICESTATUS& e, const POINT& p);

	bool IsLeftDown();
	bool IsRightDown();
	bool IsMiddleDown();

	POINT GetMouseDelta() const;
	POINT GetPosition() const;
	LONG GetXPosition() const;
	LONG GetYPosition() const;

	bool IsEmptyEventBuffer();
	bool IsEmptyRawEventBuffer();
	DXIMouseEvent ExecuteEvent();

	DXIMouseEvent ExecuteRawEvent();
	std::array<bool,(size_t)MOUSEBUTTONS::Count> m_vIsDown;
private:
	std::deque<DXIMouseEvent> eventBuffer;
	std::deque<DXIMouseEvent> rawEventBuffer;
					// 0 left 1 middle 2 right
	bool m_bLeftDown;
	bool m_bMiddleDown;
	bool m_bRightDown;
	POINT m_Position;
	POINT m_RawDelta;

};
#endif
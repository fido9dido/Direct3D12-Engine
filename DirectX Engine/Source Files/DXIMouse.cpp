#include "DXIMouse.h"

DXIMouse::DXIMouse() : m_Position{0,0}, m_bLeftDown(false), m_bMiddleDown(false), m_bRightDown(false)
{
	RAWINPUTDEVICE rid;
	rid.usUsagePage = 0x01;
	rid.usUsage = 0x02;
	rid.dwFlags = 0;
	rid.hwndTarget = 0;
	FAILED(RegisterRawInputDevices(&rid, 1, sizeof(rid)) == false);
}



POINT DXIMouse::GetMouseDelta() const
{
	return m_RawDelta;
}

POINT DXIMouse::GetPosition() const
{
	return m_Position;
}

LONG DXIMouse::GetXPosition() const
{ 
	return m_Position.x;
}

LONG DXIMouse::GetYPosition() const
{
	return m_Position.y;
}

bool DXIMouse::IsEmptyEventBuffer()
{
	return eventBuffer.empty();
}
bool DXIMouse::IsEmptyRawEventBuffer()
{
	return rawEventBuffer.empty();
}

DXIMouseEvent DXIMouse::ExecuteEvent()
{
	if (eventBuffer.empty())
	{
		return  DXIMouseEvent();
	}
	DXIMouseEvent e = eventBuffer.front();
	eventBuffer.pop_front();
	eventBuffer.shrink_to_fit();
	return e;
	
	
}
DXIMouseEvent DXIMouse::ExecuteRawEvent()
{
	if (rawEventBuffer.empty())
	{
		return  DXIMouseEvent();
	}
	DXIMouseEvent e = rawEventBuffer.front();
	rawEventBuffer.pop_front();
	rawEventBuffer.shrink_to_fit();
	return e;


}
void DXIMouse::ResetPosition()
{
	m_Position = { 0,0 };
}

void DXIMouse::OnMainButtons(const DXIMouseEvent::DEVICESTATUS& e, const POINT& p, bool val, const DXIMouseEvent::MOUSEBUTTONS& i)
{
	m_vIsDown[(int)i] = val;
	eventBuffer.emplace_back(DXIMouseEvent(e, p));
}

void DXIMouse::SetMouse(const DXIMouseEvent::DEVICESTATUS& e, const POINT& p)
{
	m_Position = p;
	eventBuffer.emplace_back(DXIMouseEvent(e, p));
}

void DXIMouse::SetRawMouse(const DXIMouseEvent::DEVICESTATUS& e, const POINT& p)
{
	m_RawDelta = p;
	rawEventBuffer.emplace_back(DXIMouseEvent(e, p));
}


bool DXIMouse::IsLeftDown()
{
	return m_vIsDown[(int)MOUSEBUTTONS::LeftB];
}

bool DXIMouse::IsRightDown()
{
	return m_vIsDown[(int)MOUSEBUTTONS::RightB];
}

bool DXIMouse::IsMiddleDown()
{
	return m_vIsDown[(int)MOUSEBUTTONS::MiddleB];
}

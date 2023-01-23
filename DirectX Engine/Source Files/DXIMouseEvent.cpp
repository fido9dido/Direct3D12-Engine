#include "DXIMouseEvent.h"

DXIMouseEvent::DXIMouseEvent() : m_DeviceStatus(DEVICESTATUS::Invalid), m_Position{ 0,0 }
{
}

DXIMouseEvent::DXIMouseEvent(const DEVICESTATUS& deviceStatus,const POINT& position) : m_DeviceStatus(deviceStatus), m_Position(position)
{
}

const DXIMouseEvent::DEVICESTATUS& DXIMouseEvent::GetMouseStatus() const
{
	return m_DeviceStatus;
}

const POINT& DXIMouseEvent::GetPosition() const
{
	return m_Position;
}

LONG DXIMouseEvent::GetXPosition() const
{
	return m_Position.x;
}

LONG DXIMouseEvent::GetYPosition() const
{
	return m_Position.y;
}

bool DXIMouseEvent::IsValid() const
{
	return m_DeviceStatus != DEVICESTATUS::Invalid;
}

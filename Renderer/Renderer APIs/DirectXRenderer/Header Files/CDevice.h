#ifndef CDEVICE_H
#define CDEVICE_H

#include "MemoryUtil.h"
// Currently not in-use
class CDevice
{

public:
private:
	unique_ptr<ID3D12Device> Device;

public:
	ID3D12Device* GetDevice() 
	{ 
		return Device.get();
	}

	void SetDevice(unique_ptr<ID3D12Device>&& device)
	{
		Device = std::move(device);
	}

private:

};
#endif
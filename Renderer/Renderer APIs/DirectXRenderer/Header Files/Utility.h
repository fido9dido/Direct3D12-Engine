#ifndef D3DUTIL_H
#define D3DUTIL_H

#include <string>
#include <wrl.h>
#include <DirectXMath.h>
#include <cmath>

#ifndef ThrowIfFailed
#define ThrowIfFailed(x)                                              \
{                                                                     \
    HRESULT hr__ = (x);                                               \
    std::wstring wfn = AnsiToWString(__FILE__);                       \
    if(FAILED(hr__)) { throw DxException(hr__, L#x, wfn, __LINE__); } \
}
#endif

class MathHelper {
public:
	static DirectX::XMFLOAT4X4 Identity4x4()
	{
		static DirectX::XMFLOAT4X4 I(
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f);

		return I;
	}

	static DirectX::XMVECTOR SphericalToCartesian(float radius, float theta, float phi)
	{
		return DirectX::XMVectorSet(
			radius * sinf(phi) * cosf(theta),
			radius * cosf(phi),
			radius * sinf(phi) * sinf(theta),
			1.0f);
	}

	static DirectX::XMVECTOR EulerAngleToQuat(float angleRadians, const DirectX::XMVECTOR& unitAxis)
	{
		assert(std::abs(1.0f - DirectX::XMVector3Length(unitAxis).m128_f32[0]) < 1e-3f);
		const float a = angleRadians * 0.5f;
		const float s = std::sin(a);

		return DirectX::XMVectorSetW(DirectX::XMVectorScale(unitAxis, s), std::cosf(a));

	}

	static DirectX::XMVECTOR EulerAngleToQuat(const DirectX::XMVECTOR& rotation)
	{
		//DirectXMath xmquaternionRotationRollPitchYaw is broken so I am implementing it like physx
		// this code rotate around z then y then x 

		DirectX::XMVECTOR qx = EulerAngleToQuat(rotation.m128_f32[0], DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f));
		DirectX::XMVECTOR qy = EulerAngleToQuat(rotation.m128_f32[1], DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
		DirectX::XMVECTOR qz = EulerAngleToQuat(rotation.m128_f32[2], DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f));

		// C++ multiplication from left to right  q1 * q2 so you expect XMQuaternionMultiply as follows 
		//  DirectX::XMQuaternionMultiply(DirectX::XMQuaternionMultiply(qz,qy), qx); but microsoft did not do it this way 
		// they implemented XMQuaternionMultiply(q1,q2) to multiply it as if XMQuaternionMultiply(q2,q1)
		//Anyway TO MULTIPLY QUATERNION AS everyone does it  qz*qy*qx it will be written as follows 

		return  DirectX::XMQuaternionMultiply(qx, DirectX::XMQuaternionMultiply(qy, qz));
	}
};

inline  UINT CalcConstantBufferByteSize(UINT byteSize)
{
	// Constant buffers must be a multiple of the minimum hardware
	// allocation size (usually 256 bytes).  So round up to nearest
	// multiple of 256.  We do this by adding 255 and then masking off
	// the lower 2 bytes which store all bits < 256.
	// Example: Suppose byteSize = 300.
	// (300 + 255) & ~255
	// 555 & ~255
	// 0x022B & ~0x00ff
	// 0x022B & 0xff00
	// 0x0200
	// 512
	return (byteSize + 255) & ~255;
}

class DxException
{
public:
	DxException() = default;
	DxException(HRESULT hr, const std::wstring& functionName, const std::wstring& filename, int lineNumber) :
		errorCode(hr),
		functionName(functionName),
		fileName(filename),
		lineNumber(lineNumber)
	{
	}

	HRESULT errorCode = S_OK;
	std::wstring functionName;
	std::wstring fileName;
	int lineNumber = -1;
};

inline std::wstring AnsiToWString(const std::string& str)
{
	WCHAR buffer[512];
	MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, buffer, 512);
	return std::wstring(buffer);
}

inline std::wstring GetFileExtensionToLower(const std::wstring& filePath)
{

	std::wstring fileName;
	size_t lastSlash;
	if ((lastSlash = filePath.rfind(L'/')) != std::string::npos)
		fileName = filePath.substr(lastSlash + 1, std::string::npos);
	else if ((lastSlash = filePath.rfind(L'\\')) != std::string::npos)
		fileName = filePath.substr(lastSlash + 1, std::string::npos);
	else
		fileName = filePath;

	size_t extOffset = fileName.rfind(L'.');
	if (extOffset == std::wstring::npos)
		return L"";

	fileName = fileName.substr(extOffset + 1);
	return fileName;


}

#endif
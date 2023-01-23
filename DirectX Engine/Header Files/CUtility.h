#ifndef CUTILITY_H
#define CUTILITY_H

#include <DirectXMath.h>
#include <cmath>

class CUtility
{
public:
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

	static DirectX::XMVECTOR EulerAngleToQuat(float angleRadians, const DirectX::XMVECTOR& unitAxis)
	{
		assert(std::abs(1.0f - DirectX::XMVector3Length(unitAxis).m128_f32[0]) < 1e-3f);
		const float a = angleRadians * 0.5f;
		const float s = std::sin(a);

		return DirectX::XMVectorSetW(DirectX::XMVectorScale(unitAxis, s), std::cosf(a));

	}
};

#endif
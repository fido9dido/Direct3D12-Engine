#include "MovementController.h"
#include "CUtility.h"

//MovementController
MovementController::MovementController(void) :
	Position(DirectX::g_XMZero),
	RightVector(DirectX::g_XMIdentityR0),
	UpVector(DirectX::g_XMIdentityR1),
	DirectionVector(DirectX::g_XMIdentityR2),
	Velocity(DirectX::g_XMZero),
	PitchSpeed(0.0f),
	YawSpeed(0.0f),
	QuaternionVector(DirectX::g_XMIdentityR3),
	Rotation(DirectX::g_XMZero),
	YawSpeedMax(10.f),
	PitchSpeedMax(0.0f),
	RollSpeed(0)
{
}

MovementController::~MovementController(void)
{
}
void MovementController::SetRotationSpeedX(float f)
{
	PitchSpeed = f;
	bViewDirty = true;
}
void MovementController::SetRotationSpeedY(float f)
{
	YawSpeed = f;
	bViewDirty = true;
}
void MovementController::SetRotation(float rx, float ry, float rz)
{
	Rotation = DirectX::XMVectorSet(rx, ry, rz, 0);
	
	//RecalcAxes();
	 bViewDirty = true;
}

void MovementController::SetPosition(DirectX::XMVECTOR& vc)
{
	memcpy(&Position, &vc, sizeof(DirectX::XMVECTOR));
}

void MovementController::SetRightVector(DirectX::XMVECTOR& vc)
{
	memcpy(&RightVector, &vc, sizeof(DirectX::XMVECTOR));
}

void MovementController::SetUpVector(DirectX::XMVECTOR& vector)
{
	memcpy(&UpVector, &vector, sizeof(DirectX::XMVECTOR));
}

void MovementController::SetDirectionVector(DirectX::XMVECTOR& vc)
{
	memcpy(&DirectionVector, &vc, sizeof(DirectX::XMVECTOR));
}

/**
 * Recalculate the orientation of the local axes according to the
 * current rotation values around the axes.
 */
void MovementController::RecalcAxes(void)
{
	//Quaternion
	DirectX::XMVECTOR quaternionFrame;

	// keep in range of 360 degree
	if (Rotation.m128_f32[0] > DirectX::XM_2PI)
	{
		Rotation.m128_f32[0] -= DirectX::XM_2PI;
	}
	else if (Rotation.m128_f32[0] < -DirectX::XM_2PI)
	{
		Rotation.m128_f32[0] += DirectX::XM_2PI;
	}
	if (Rotation.m128_f32[1] > DirectX::XM_2PI)
	{
		Rotation.m128_f32[1] -= DirectX::XM_2PI;
	}
	else if (Rotation.m128_f32[1] < -DirectX::XM_2PI)
	{
		Rotation.m128_f32[1] += DirectX::XM_2PI;
	}
	if (Rotation.m128_f32[2] > DirectX::XM_2PI)
	{
		Rotation.m128_f32[2] -= DirectX::XM_2PI;
	}
	else if (Rotation.m128_f32[2] < -DirectX::XM_2PI)
	{
		Rotation.m128_f32[2] += DirectX::XM_2PI;
	}

	quaternionFrame = CUtility::EulerAngleToQuat(Rotation);
	
	//multiply to add rotation to  current quat rotation
	QuaternionVector = DirectX::XMQuaternionMultiply(quaternionFrame, QuaternionVector);
	
	// extract the local axes rot matrix
	DirectX::XMMATRIX matrix = DirectX::XMMatrixRotationQuaternion(QuaternionVector);
	
	RightVector = matrix.r[0];
	UpVector = matrix.r[1];
	DirectionVector = matrix.r[2];;

}


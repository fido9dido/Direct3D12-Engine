#include "FirstPersonMode.h"

FirstPersonMode::FirstPersonMode(void): Speed(0), Slide(0), MovementController()
{
}

void FirstPersonMode::Update(float fDeltaTime)
{
	if (bViewDirty) {

		// add rotation speed
		Rotation = DirectX::XMVectorAdd(Rotation,DirectX::XMVectorScale(DirectX::XMVectorSet(PitchSpeed, YawSpeed, RollSpeed,0), fDeltaTime));
		
		// recalculate axes
		RecalcAxes();

		// calculate velocity vectors
		Velocity = DirectX::XMVectorScale(DirectionVector, Speed * fDeltaTime);
		DirectX::XMVECTOR vcSpeed = DirectX::XMVectorScale(RightVector, Slide * fDeltaTime);

		// move position according to velocity vector
		Position = DirectX::XMVectorAdd(Position,DirectX::XMVectorAdd(Velocity, vcSpeed));
		bViewDirty = false;
		PitchSpeed = 0.f;
		YawSpeed = 0.f;
		RollSpeed = 0.f;
	}
}

const DirectX::XMVECTOR& FirstPersonMode::GetRotation(void)
{
	return Rotation;
}

void FirstPersonMode::SetSpeed(float a)
{
	Speed = a;  
	Slide = 0;
	bViewDirty = true;
}

void FirstPersonMode::SetSlideSpeed(float a)
{
	Slide = a;
	Speed = 0;
	bViewDirty = true;
}

void FirstPersonMode::SetRotation(float rx, float ry, float rz)
{
	Rotation =  DirectX::XMVectorSet(rx,ry,rz,0);
	
	RecalcAxes();
}

void FirstPersonMode::RecalcAxes(void)
{
	DirectX::XMMATRIX  matrix;

	static float f2PI = 6.283185f;

	// keep in range of 360 degree
	if (Rotation.m128_f32[1] > f2PI) Rotation.m128_f32[1] -= f2PI;
	else if (Rotation.m128_f32[1] < -f2PI) Rotation.m128_f32[1] += f2PI;

	// dont look up/down more than 80 degrees
	if (Rotation.m128_f32[0] > 1.4f) Rotation.m128_f32[0] = 1.4f;
	else if (Rotation.m128_f32[0] < -1.4f) Rotation.m128_f32[0] = -1.4f;

	// initialize axes
	RightVector = DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0);
	UpVector = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0);
	DirectionVector = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0);
	
	// rotate around Y axis
	matrix = DirectX::XMMatrixRotationAxis(UpVector, Rotation.m128_f32[1]);
	// multiply  vector and matrix
	RightVector = DirectX::XMVector3Transform(RightVector, matrix);
	DirectionVector = DirectX::XMVector3Transform(DirectionVector, matrix);

	// rotate around X axis
	matrix = DirectX::XMMatrixRotationAxis(RightVector, Rotation.m128_f32[0]);
	UpVector = DirectX::XMVector3Transform(UpVector, matrix);
	DirectionVector = DirectX::XMVector3Transform(DirectionVector, matrix);
	
	// fight rounding errors
	DirectionVector = DirectX::XMVector3Normalize(DirectionVector);
	RightVector = DirectX::XMVector3Cross(UpVector, DirectionVector);
	RightVector = DirectX::XMVector3Normalize(RightVector);
	UpVector = DirectX::XMVector3Cross(DirectionVector, RightVector);
	UpVector = DirectX::XMVector3Normalize(UpVector);
}

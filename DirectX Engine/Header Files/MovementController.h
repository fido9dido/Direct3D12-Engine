#ifndef MOVEMENTCONTROLLER_H
#define MOVEMENTCONTROLLER_H

#include <DirectXMath.h>

//todo reset mouse coord to center per frame
class MovementController
{
public:
	MovementController(void);
	virtual ~MovementController(void);
	virtual void Update(float fElapsedTime) = 0;
	// Accessor-Methods
	DirectX::XMVECTOR  GetPosition(void) { return Position; }
	DirectX::XMVECTOR  GetRightVector(void) { return RightVector; }
	DirectX::XMVECTOR  GetUpVector(void) { return UpVector; }
	DirectX::XMVECTOR  GetDirectionVector(void) { return DirectionVector; }
	DirectX::XMVECTOR  GetVelocity(void) { return Velocity; }

	void SetRotationSpeedX(float f);
	void SetRotationSpeedY(float f);


	//direct access 
	virtual void SetRotation(float rx, float ry, float rz);
	void SetPosition(DirectX::XMVECTOR& vc);
	void SetRightVector(DirectX::XMVECTOR& vc);
	void SetUpVector(DirectX::XMVECTOR& vc);
	void SetDirectionVector(DirectX::XMVECTOR& vc);

protected:
	bool bViewDirty = true;
	DirectX::XMVECTOR Position; // position
	DirectX::XMVECTOR RightVector; // right vector
	DirectX::XMVECTOR UpVector; // up vector
	DirectX::XMVECTOR DirectionVector; // direction vector
	DirectX::XMVECTOR Velocity; // speed vector
	DirectX::XMVECTOR QuaternionVector; // quaternion for rotation
	
	// rotation speed on local axis
	float RollSpeed;
	float PitchSpeed;
	float YawSpeed;

	//MAX speed
	float PitchSpeedMax;
	float YawSpeedMax;

	// rotation angle on local axis
	DirectX::XMVECTOR Rotation;

	virtual void RecalcAxes(void);

};
#endif

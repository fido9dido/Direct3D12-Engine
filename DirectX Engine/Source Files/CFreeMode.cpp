#include "CFreeMode.h"

CFreeMode::CFreeMode():MovementController() , Thrust(1)
{

}

CFreeMode::~CFreeMode(void)
{

}

void CFreeMode::AddRotationSpeed(float speedX, float speedY, float speedZ)
{
	
	PitchSpeed += speedX;
	YawSpeed += speedY;
	RollSpeed += speedZ;
}

void CFreeMode::SetRotationSpeed(float speedX, float speedY, float speedZ)
{
	
	Rotation = DirectX::XMVectorSet(speedX, speedY,  speedZ, 0);
	bViewDirty = true;
}

/**
 * Calculate movement based on behavior model and attribute values.
 * Input parameter is elapsed time since last frame.
 */
void CFreeMode::Update(float fElapsedTime) {
	// add rotation speed
	if (bViewDirty) {

		Rotation = DirectX::XMVectorScale(Rotation, fElapsedTime);
		
		// calculate velocity vector
		Velocity = DirectX::XMVectorScale(DirectionVector, Thrust * fElapsedTime);

		// move position according to velocity vector
		Position = DirectX::XMVectorAdd(Position, Velocity);

		// recalculate axes
		RecalcAxes();
		bViewDirty = false;
	}
} // Update
/*----------------------------------------------------------------*/
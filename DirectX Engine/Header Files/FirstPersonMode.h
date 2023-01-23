#ifndef FIRSTPERSONMODE_H
#define FIRSTPERSONMODE_H

#include "MovementController.h"

class FirstPersonMode : public MovementController
{
public: 

	FirstPersonMode(void);

    const DirectX::XMVECTOR& GetRotation(void);

    virtual void Update(float fElapsedTime);
    void SetSpeed(float a);
    void SetSlideSpeed(float a);
    void SetRotation(float rx, float ry, float rz);

private:
    void RecalcAxes(void);

     float Speed;
     float Slide;
};
#endif
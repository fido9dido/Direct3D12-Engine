#ifndef DXRCAMERA_H
#define DXRCAMERA_H

#include "MovementController.h"

class CFreeMode :public MovementController
{
public:
    CFreeMode();
    virtual ~CFreeMode(void);

    void AddRotationSpeed(float sx, float sy, float sz);
    void SetRotationSpeed(float sx, float sy, float sz);
    void SetRotationSpeedZ(float f) { RollSpeed = f; }
    void AddThrust(float fThrust) { Thrust += fThrust; }
    void SetThrust(float fThrust) { Thrust = fThrust; }
    void Update(float fElapsedTime);
private:
    float Thrust;
    float RollSpeed;
    float RollSpeedMax;

};
#endif
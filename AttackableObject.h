#pragma once

#include "Object.h"
#include "Bullet.h"

class AttackableObject : public Object
{
public:
    AttackableObject(const D2D1_POINT_2F pos, ID2D1Bitmap* pImage, Bullet* pBullet);
    virtual ~AttackableObject();
    AttackableObject(const AttackableObject& other) = default;
    AttackableObject& operator=(const AttackableObject& other) = default;

    virtual void Attack() = 0;

    Bullet* GetBullet();

protected:
    Bullet* mpBullet;
};
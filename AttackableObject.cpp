#include "AttackableObject.h"

AttackableObject::AttackableObject(const D2D1_POINT_2F pos, ID2D1Bitmap* pImage, Bullet* pBullet)
    : Object(pos, pImage)
    , mpBullet(pBullet)
{

}

AttackableObject::~AttackableObject()
{
    delete mpBullet;
}

Bullet* AttackableObject::GetBullet()
{
    return mpBullet;
}

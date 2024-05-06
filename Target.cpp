#include "Target.h"

#define RESET_VALUE (-1000.f)

Target::Target(ID2D1Bitmap* pImage, Bullet* pBullet)
    : AttackableObject({ RESET_VALUE, RESET_VALUE }, pImage, pBullet)
    , mHitSound(TEXT("./Data/Sound/Damage.mp3"))
{

}

void Target::Draw(ID2D1RenderTarget& renderTarget)
{
    if (IsDead())
    {
        return;
    }

    D2D1_RECT_F rt = GetImageRect(
        AttackableObject::Object::mPosition,
        TARGET_DIST_FROM_CENTER_TO_SIDE
    );

    renderTarget.DrawBitmap(
        AttackableObject::Object::mpImage,
        rt
    );
}

void Target::OnCollision()
{
    mHitSound.PlayAudio();

    AttackableObject::Object::mPosition.x = RESET_VALUE;
    AttackableObject::Object::mPosition.y = RESET_VALUE;
}

inline D2D1_RECT_F Target::GetCollisionRect()
{
    D2D1_RECT_F rect = GetImageRect(AttackableObject::Object::mPosition, TARGET_DIST_FROM_CENTER_TO_SIDE);

    return rect;
}

void Target::Attack()
{
    if (IsDead())
    {
        return;
    }

    if (!AttackableObject::mpBullet->IsDead())
    {
        return;
    }

    D2D1_POINT_2F newPos;
    newPos.x = AttackableObject::Object::mPosition.x;
    newPos.y = AttackableObject::Object::mPosition.y + TARGET_DIST_FROM_CENTER_TO_SIDE;

    mpBullet->Revive(newPos);
}

bool Target::IsDead()
{
    return AttackableObject::Object::mPosition.x < 0.f;
}

void Target::Revive(const D2D1_POINT_2F newPos)
{
    ResetPosition(newPos);
}

void Target::ResetPosition(const D2D1_POINT_2F newPos)
{
    AttackableObject::Object::mPosition = newPos;
}

void Target::ResetPosition()
{
    ResetPosition({ RESET_VALUE, RESET_VALUE });
}

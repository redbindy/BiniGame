#include "Player.h"

enum EShapeInfo
{
    DIST_FROM_CENTER_TO_SIDE = 75
};

Player::Player(ID2D1Bitmap* pImage, Bullet* pBullet)
    : AttackableObject({ SCREEN_WIDTH / (float)2, SCREEN_HEIGHT - 110.f }, pImage, pBullet)
    , mAttackSound(TEXT("./Data/Sound/Attack.mp3"))
    , mHitSound(TEXT("./Data/Sound/Hit.mp3"))
    , bLeftDown(false)
    , bRightDown(false)
    , bSpaceDown(false)
{
    mAttackSound.TurnVolumeDown(-1000);
}

void Player::Draw(ID2D1RenderTarget& renderTarget)
{
    D2D1_RECT_F rt = GetImageRect(
        AttackableObject::Object::mPosition,
        EShapeInfo::DIST_FROM_CENTER_TO_SIDE
    );

    renderTarget.DrawBitmap(
        Object::mpImage,
        rt
    );
}

void Player::OnCollision()
{
    mHitSound.PlayAudio();
}

inline D2D1_RECT_F Player::GetCollisionRect()
{
    D2D1_RECT_F rect = GetImageRect(AttackableObject::Object::mPosition, DIST_FROM_CENTER_TO_SIDE / 10);

    return rect;
}

void Player::Attack()
{
    if (!bSpaceDown)
    {
        return;
    }

    if (!AttackableObject::mpBullet->IsDead())
    {
        return;
    }

    D2D1_POINT_2F newPos;
    newPos.x = AttackableObject::Object::mPosition.x;
    newPos.y = AttackableObject::Object::mPosition.y - DIST_FROM_CENTER_TO_SIDE;

    mpBullet->Revive(newPos);

    mAttackSound.PlayAudio();
}

void Player::Move()
{
#define DELTA_MOVE (5.f);

    if (bLeftDown)
    {
        if (AttackableObject::Object::mPosition.x - DIST_FROM_CENTER_TO_SIDE <= 0)
        {
            AttackableObject::Object::mPosition.x = DIST_FROM_CENTER_TO_SIDE;
        }
        else
        {
            AttackableObject::Object::mPosition.x -= DELTA_MOVE;
        }
    }

    if (bRightDown)
    {
        if (AttackableObject::Object::mPosition.x + DIST_FROM_CENTER_TO_SIDE >= SCREEN_WIDTH)
        {
            AttackableObject::Object::mPosition.x = SCREEN_WIDTH - DIST_FROM_CENTER_TO_SIDE;
        }
        else
        {
            AttackableObject::Object::mPosition.x += DELTA_MOVE;
        }
    }
}

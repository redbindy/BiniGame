#include "Bullet.h"

#define RESET_VALUE (-1000.f)

enum EShapeInfo
{
    DIST_FROM_CENTER_TO_SIDE = 18
};

Bullet::Bullet(ID2D1Bitmap* pImage, const float delta)
    : Object({ RESET_VALUE, RESET_VALUE }, pImage)
    , DELTA_MOVE(delta)
{

}

void Bullet::Draw(ID2D1RenderTarget& renderTarget)
{
    if (IsDead())
    {
        return;
    }

    const float DELTA_THETA = 1.f;
    static float theta = 0.f;

    theta += DELTA_THETA;

    D2D1_MATRIX_3X2_F rotationMat = D2D1::Matrix3x2F::Rotation(
        theta,
        Object::mPosition
    );

    renderTarget.SetTransform(
        rotationMat
    );

    D2D1_RECT_F rt = GetImageRect(
        Object::mPosition,
        DIST_FROM_CENTER_TO_SIDE
    );

    renderTarget.DrawBitmap(
        Object::mpImage,
        rt
    );

    renderTarget.SetTransform(
        D2D1::IdentityMatrix()
    );
}

void Bullet::OnCollision()
{
    Object::mPosition.x = RESET_VALUE;
    Object::mPosition.y = RESET_VALUE;
}

void Bullet::Move()
{
    if (IsDead())
    {
        return;
    }

    Object::mPosition.y += DELTA_MOVE;

    if (IsDead())
    {
        Object::mPosition.x = RESET_VALUE;
        Object::mPosition.y = RESET_VALUE;
    }
}

inline D2D1_RECT_F Bullet::GetCollisionRect()
{
    D2D1_RECT_F rect = GetImageRect(Object::mPosition, DIST_FROM_CENTER_TO_SIDE);

    return rect;
}

bool Bullet::IsDead()
{
    return Object::mPosition.y < 0 || Object::mPosition.y > SCREEN_HEIGHT;
}

void Bullet::Revive(const D2D1_POINT_2F pos)
{
    Object::mPosition = pos;
}


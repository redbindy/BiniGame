#pragma once

#include "framework.h"
#include "Object.h"
#include "IMovable.h"

class Bullet final : public Object, public IMovable
{
public:
    Bullet(ID2D1Bitmap* pImage, const float delta);
    ~Bullet() = default;
    Bullet(const Bullet& other) = default;
    Bullet& operator=(const Bullet& other) = default;

    virtual void Draw(ID2D1RenderTarget& renderTarget) override;
    virtual void OnCollision() override;
    virtual void Move() override;
    virtual D2D1_RECT_F GetCollisionRect() override;

    bool IsDead();

    void Revive(const D2D1_POINT_2F attackerPosition);

private:
    const float DELTA_MOVE;
};

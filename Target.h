#pragma once

#include "AttackableObject.h"
#include "AudioController.h"

enum EShapeInfo
{
    TARGET_DIST_FROM_CENTER_TO_SIDE = 75
};

class Target final : public AttackableObject
{
public:
    Target(ID2D1Bitmap* pImage, Bullet* pBullet);
    ~Target() = default;
    Target(const Target& other) = default;
    Target& operator=(const Target& other) = default;

    virtual void Draw(ID2D1RenderTarget& renderTarget) override;
    virtual void OnCollision() override;
    inline D2D1_RECT_F GetCollisionRect() override;
    virtual void Attack() override;

    bool IsDead();
    void Revive(const D2D1_POINT_2F newPos);
    void ResetPosition(const D2D1_POINT_2F newPos);
    void ResetPosition();

private:
    AudioController mHitSound;
};
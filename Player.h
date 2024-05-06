#pragma once

#include "AttackableObject.h"
#include "AudioController.h"
#include "IMovable.h"

class Player final : public AttackableObject, public IMovable
{
public:
    Player(ID2D1Bitmap* pImage, Bullet* pBullet);
    ~Player() = default;
    Player(const Player& other) = delete;
    Player& operator=(const Player& other) = delete;

    virtual void Draw(ID2D1RenderTarget& renderTarget) override;
    virtual void OnCollision() override;
    virtual D2D1_RECT_F GetCollisionRect() override;

    virtual void Attack() override;
    virtual void Move() override;

    bool bLeftDown;
    bool bRightDown;
    bool bSpaceDown;

private:
    AudioController mAttackSound;
    AudioController mHitSound;
};
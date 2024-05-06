#pragma once

#include <d2d1.h>
#pragma comment(lib, "d2d1.lib")

#include "ComHelper.h"
#include "DrawHelper.h"

class Object
{
public:
    Object(const D2D1_POINT_2F pos, ID2D1Bitmap* pImage);
    virtual ~Object();
    Object(const Object& other) = default;
    Object& operator=(const Object& other) = default;

    virtual void Draw(ID2D1RenderTarget& renderTarget) = 0;
    virtual void OnCollision() = 0;

    inline D2D1_POINT_2F GetPosition();
    virtual D2D1_RECT_F GetCollisionRect() = 0;

protected:
    D2D1_POINT_2F mPosition;
    ID2D1Bitmap* mpImage;
};

inline D2D1_POINT_2F Object::GetPosition()
{
    return mPosition;
}

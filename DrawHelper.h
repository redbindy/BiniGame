#pragma once

#include "d2d1.h"

inline D2D1_RECT_F GetImageRect(const D2D1_POINT_2F pos, const float dist)
{
    D2D1_RECT_F rect;
    rect.left = pos.x - dist;
    rect.top = pos.y - dist;
    rect.right = pos.x + dist;
    rect.bottom = pos.y + dist;

    return rect;
}
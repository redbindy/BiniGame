#include "Object.h"

Object::Object(const D2D1_POINT_2F pos, ID2D1Bitmap* pImage)
    : mPosition(pos)
    , mpImage(pImage)
{

}

Object::~Object()
{
    COM_RELEASE(mpImage);
}

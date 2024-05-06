#pragma once

template<class C> void COM_RELEASE(C*& objPtr)
{
    if (objPtr != nullptr)
    {
        objPtr->Release();
        objPtr = nullptr;
    }
}
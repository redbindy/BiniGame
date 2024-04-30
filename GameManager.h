#pragma once

#include "framework.h"

#include "d2d1.h"
#pragma comment(lib, "d2d1.lib")

namespace GameManager
{
	void Initialize(const HWND hWnd);
	void Destroy();

	void Render();

	void OnKeyDown(const UINT keyCode);
}
#pragma once

#include "framework.h"

#include <d2d1.h>
#pragma comment(lib, "d2d1.lib")

#include <wincodec.h>
#pragma comment(lib, "windowscodecs")

#include <dshow.h>
#pragma comment(lib, "strmiids.lib")

#include <random>
#include <vector>
#include <cmath>

namespace GameManager
{
	void Initialize(const HWND hWnd);
	void Destroy();

	void Run();
	static void Render();

	void OnKeyDown(const UINT keyCode);
	void OnKeyUp(const UINT keyCode);
}
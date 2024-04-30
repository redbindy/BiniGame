#include "GameManager.h"

static HWND shWnd = nullptr;

static ID2D1Factory* spFactory = nullptr;
static ID2D1HwndRenderTarget* sphWndRenderTarget = nullptr;

void GameManager::Initialize(const HWND hWnd)
{
	shWnd = hWnd;

	HRESULT hr = D2D1CreateFactory(
		D2D1_FACTORY_TYPE_SINGLE_THREADED,
		&spFactory
	);
	assert(SUCCEEDED(hr));

	RECT rt;
	GetClientRect(hWnd, &rt);

	hr = spFactory->CreateHwndRenderTarget(
		D2D1::RenderTargetProperties(),
		D2D1::HwndRenderTargetProperties(shWnd, D2D1::SizeU(rt.right, rt.bottom)),
		&sphWndRenderTarget
	);
	assert(SUCCEEDED(hr));
}

template<class C> void D2D_RELEASE(C*& obj)
{
	if (obj != nullptr)
	{
		obj->Release();
		obj = nullptr;
	}
}

void GameManager::Destroy()
{
	D2D_RELEASE(sphWndRenderTarget);
	D2D_RELEASE(spFactory);
}

static D2D_RECT_F rt = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, SCREEN_WIDTH / 2 + 30.f, SCREEN_HEIGHT / 2 + 30.f };

void GameManager::Render()
{
	assert(shWnd != nullptr);
	assert(sphWndRenderTarget != nullptr);

	sphWndRenderTarget->BeginDraw();
	{
		sphWndRenderTarget->Clear(
			D2D1::ColorF(D2D1::ColorF::White)
		);

		ID2D1SolidColorBrush* brush = nullptr;
		sphWndRenderTarget->CreateSolidColorBrush(
			D2D1::ColorF(D2D1::ColorF::Pink),
			&brush
		);
		assert(brush != nullptr);

		sphWndRenderTarget->FillRectangle(
			rt, 
			brush
		);
		brush->Release();
	}
	sphWndRenderTarget->EndDraw();
}

void OnKeyDown(const UINT keyCode)
{
	switch (keyCode)
	{
	case VK_LEFT:

		if (rt.left - 3.f < 0)
		{
			rt.left = 0.f;
			rt.right = 10.f;
		}
		else
		{
			rt.left -= 3.f;
			rt.right -= 3.f;
		}

		break;

	case VK_RIGHT:
		break;

	case VK_UP:
		break;

	case VK_DOWN:
		break;

	default:
		return;
	}
}
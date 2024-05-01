#include "GameManager.h"

static HWND shWnd = nullptr;

static ID2D1HwndRenderTarget* sphWndRenderTarget = nullptr;

static ID2D1Bitmap* spPlayerBitmap = nullptr;
static ID2D1Bitmap* spPlayerBulletBitmap = nullptr;

static ID2D1Bitmap* spTargetBitmap = nullptr;
static ID2D1Bitmap* spTargetBulletBitmap = nullptr;

static IGraphBuilder* spAttackSoundGraph = nullptr;
static IMediaControl* spAttackSoundControl = nullptr;
static IMediaPosition* spAttackSoundPosition = nullptr;
static IMediaEvent* spAttackSoundEvent = nullptr;
static IBasicAudio* spAttackSoundAudio = nullptr;

static IGraphBuilder* spHitSoundGraph = nullptr;
static IMediaControl* spHitSoundControl = nullptr;
static IMediaPosition* spHitSoundPosition = nullptr;
static IMediaEvent* spHitSoundEvent = nullptr;

static IGraphBuilder* spDamageSoundGraph = nullptr;
static IMediaControl* spDamageSoundControl = nullptr;
static IMediaPosition* spDamageSoundPosition = nullptr;
static IMediaEvent* spDamageSoundEvent = nullptr;

static std::random_device sRandomDevice;
static std::mt19937_64 gen(sRandomDevice());
static std::uniform_real_distribution<float> random(0.f, 1.f);

enum eShapeInformation
{
    OBJECT_DIST_FROM_CENTER_TO_SIDE = 75,

    BULLET_DIST_FROM_CENTER_TO_SIDE = 18
};

static D2D1_POINT_2F sPlayerPos = { SCREEN_WIDTH / (float)2, SCREEN_HEIGHT - 110.f };
static D2D1_POINT_2F sPlayerBulletPos = { sPlayerPos.x, -1.f };

static bool bLeftDown = false;
static bool bRightDown = false;
static bool bSpaceDown = false;

enum
{
    TARGET_COUNT = 3
};

static D2D1_POINT_2F sTargetPosArr[TARGET_COUNT] = { 0, };
static D2D1_POINT_2F sTargetBulletPosArr[TARGET_COUNT] = { 0, };
static bool sbAlive[TARGET_COUNT] = { false, };

void loadImage(const TCHAR* filePath, ID2D1Bitmap** outBitmapPtr);
D2D1_RECT_F getImageRect(const D2D1_POINT_2F pos, const float dist);
static D2D1_POINT_2F generateTargetPos();
static void spawnTargets();
static bool CheckCollision(const D2D1_RECT_F rect1, const D2D1_RECT_F rect2);

template<class C> void COM_RELEASE(C*& objPtr)
{
    if (objPtr != nullptr)
    {
        objPtr->Release();
        objPtr = nullptr;
    }
}

void GameManager::Initialize(const HWND hWnd)
{
    shWnd = hWnd;

    ID2D1Factory* pD2DFactory = nullptr;
    {
        HRESULT hr = D2D1CreateFactory(
            D2D1_FACTORY_TYPE_SINGLE_THREADED,
            &pD2DFactory
        );
        assert(SUCCEEDED(hr));

        RECT rt;
        GetClientRect(hWnd, &rt);

        hr = pD2DFactory->CreateHwndRenderTarget(
            D2D1::RenderTargetProperties(),
            D2D1::HwndRenderTargetProperties(shWnd, D2D1::SizeU(rt.right, rt.bottom)),
            &sphWndRenderTarget
        );
        assert(SUCCEEDED(hr));

        hr = CoInitializeEx(
            nullptr,
            COINIT_APARTMENTTHREADED
        );
        assert(SUCCEEDED(hr));

        loadImage(TEXT("./Data/Image/Bini.jpg"), &spPlayerBitmap);
        loadImage(TEXT("./Data/Image/Heart.png"), &spPlayerBulletBitmap);

        loadImage(TEXT("./Data/Image/Bokdang.jpg"), &spTargetBitmap);
        loadImage(TEXT("./Data/Image/Cheese.png"), &spTargetBulletBitmap);
    }
    COM_RELEASE(pD2DFactory);

    // ���� �Ҹ�
    {
        HRESULT hr = CoCreateInstance(
            CLSID_FilterGraph,
            nullptr,
            CLSCTX_INPROC_SERVER,
            IID_IGraphBuilder,
            (void**)&spAttackSoundGraph
        );
        assert(SUCCEEDED(hr));

        hr = spAttackSoundGraph->QueryInterface(
            IID_IMediaControl,
            (void**)&spAttackSoundControl
        );
        assert(SUCCEEDED(hr));

        hr = spAttackSoundGraph->QueryInterface(
            IID_IMediaPosition,
            (void**)&spAttackSoundPosition
        );
        assert(SUCCEEDED(hr));

        hr = spAttackSoundGraph->QueryInterface(
            IID_IMediaEvent,
            (void**)&spAttackSoundEvent
        );
        assert(SUCCEEDED(hr));

        hr = spAttackSoundGraph->QueryInterface(
            IID_IBasicAudio,
            (void**)&spAttackSoundAudio
        );
        assert(SUCCEEDED(hr));

        hr = spAttackSoundGraph->RenderFile(TEXT("./Data/Sound/Attack.mp3"), nullptr);
        assert(SUCCEEDED(hr));

        spAttackSoundAudio->put_Volume(-1000);
    }

    // �ǰ� �Ҹ�
    {
        HRESULT hr = CoCreateInstance(
            CLSID_FilterGraph,
            nullptr,
            CLSCTX_INPROC_SERVER,
            IID_IGraphBuilder,
            (void**)&spHitSoundGraph
        );
        assert(SUCCEEDED(hr));

        hr = spHitSoundGraph->QueryInterface(
            IID_IMediaControl,
            (void**)&spHitSoundControl
        );
        assert(SUCCEEDED(hr));

        hr = spHitSoundGraph->QueryInterface(
            IID_IMediaPosition,
            (void**)&spHitSoundPosition
        );
        assert(SUCCEEDED(hr));

        hr = spHitSoundGraph->QueryInterface(
            IID_IMediaEvent,
            (void**)&spHitSoundEvent
        );
        assert(SUCCEEDED(hr));

        hr = spHitSoundGraph->RenderFile(TEXT("./Data/Sound/Hit.mp3"), nullptr);
        assert(SUCCEEDED(hr));
    }

    // Ÿ�� �Ҹ�
    {
        HRESULT hr = CoCreateInstance(
            CLSID_FilterGraph,
            nullptr,
            CLSCTX_INPROC_SERVER,
            IID_IGraphBuilder,
            (void**)&spDamageSoundGraph
        );
        assert(SUCCEEDED(hr));

        hr = spDamageSoundGraph->QueryInterface(
            IID_IMediaControl,
            (void**)&spDamageSoundControl
        );
        assert(SUCCEEDED(hr));

        hr = spDamageSoundGraph->QueryInterface(
            IID_IMediaPosition,
            (void**)&spDamageSoundPosition
        );
        assert(SUCCEEDED(hr));

        hr = spDamageSoundGraph->QueryInterface(
            IID_IMediaEvent,
            (void**)&spDamageSoundEvent
        );
        assert(SUCCEEDED(hr));

        hr = spDamageSoundGraph->RenderFile(TEXT("./Data/Sound/Damage.mp3"), nullptr);
        assert(SUCCEEDED(hr));
    }

    spawnTargets();
}

void GameManager::Destroy()
{
    CoUninitialize();

    COM_RELEASE(spHitSoundEvent);
    COM_RELEASE(spHitSoundPosition);
    COM_RELEASE(spHitSoundControl);
    COM_RELEASE(spHitSoundGraph);

    COM_RELEASE(spAttackSoundEvent);
    COM_RELEASE(spAttackSoundPosition);
    COM_RELEASE(spAttackSoundControl);
    COM_RELEASE(spAttackSoundGraph);

    COM_RELEASE(spTargetBulletBitmap);
    COM_RELEASE(spTargetBitmap);
    COM_RELEASE(spPlayerBulletBitmap);
    COM_RELEASE(spPlayerBitmap);

    COM_RELEASE(sphWndRenderTarget);
}

void loadImage(const TCHAR* filePath, ID2D1Bitmap** outBitmapPtr)
{
    assert(filePath != nullptr);
    assert(outBitmapPtr != nullptr);

    IWICImagingFactory* pImageFactory = nullptr;
    IWICBitmapDecoder* pDecoder = nullptr;
    IWICBitmapFrameDecode* pFrame = nullptr;
    IWICFormatConverter* pConverter = nullptr;
    {
        HRESULT hr = CoCreateInstance(
            CLSID_WICImagingFactory,
            nullptr,
            CLSCTX_INPROC_SERVER,
            IID_PPV_ARGS(&pImageFactory)
        );
        assert(SUCCEEDED(hr));

        hr = pImageFactory->CreateDecoderFromFilename(
            filePath,
            nullptr,
            GENERIC_READ,
            WICDecodeMetadataCacheOnDemand,
            &pDecoder
        );
        assert(SUCCEEDED(hr));

        hr = pDecoder->GetFrame(0, &pFrame);
        assert(SUCCEEDED(hr));

        hr = pImageFactory->CreateFormatConverter(&pConverter);
        assert(SUCCEEDED(hr));

        hr = pConverter->Initialize(
            pFrame,
            GUID_WICPixelFormat32bppPBGRA,
            WICBitmapDitherTypeNone,
            nullptr,
            0.f,
            WICBitmapPaletteTypeCustom
        );
        assert(SUCCEEDED(hr));

        hr = sphWndRenderTarget->CreateBitmapFromWicBitmap(
            pConverter,
            nullptr,
            outBitmapPtr
        );
        assert(SUCCEEDED(hr));
    }
    COM_RELEASE(pConverter);
    COM_RELEASE(pFrame);
    COM_RELEASE(pDecoder);
    COM_RELEASE(pImageFactory);
}

const float DELTA_MOVE = 5.f;
const float DELTA_PLAYER_BULLET = 10.f;
const float DELTA_TARGET_BULLET = 2.f;

void GameManager::Run()
{
    if (bLeftDown)
    {
        if (sPlayerPos.x - OBJECT_DIST_FROM_CENTER_TO_SIDE <= 0)
        {
            sPlayerPos.x = OBJECT_DIST_FROM_CENTER_TO_SIDE;
        }
        else
        {
            sPlayerPos.x -= DELTA_MOVE;
        }
    }

    if (bRightDown)
    {
        if (sPlayerPos.x + OBJECT_DIST_FROM_CENTER_TO_SIDE >= SCREEN_WIDTH)
        {
            sPlayerPos.x = SCREEN_WIDTH - OBJECT_DIST_FROM_CENTER_TO_SIDE;
        }
        else
        {
            sPlayerPos.x += DELTA_MOVE;
        }
    }

    if (bSpaceDown && sPlayerBulletPos.y < 0)
    {
        sPlayerBulletPos.x = sPlayerPos.x;
        sPlayerBulletPos.y = sPlayerPos.y - OBJECT_DIST_FROM_CENTER_TO_SIDE;

        spAttackSoundPosition->put_CurrentPosition(0);
        spAttackSoundControl->Run();
    }

    sPlayerBulletPos.y -= DELTA_PLAYER_BULLET;

    D2D1_RECT_F playerRect = getImageRect(sPlayerPos, OBJECT_DIST_FROM_CENTER_TO_SIDE / 10);
    D2D1_RECT_F playerBulletRect = getImageRect(sPlayerBulletPos, BULLET_DIST_FROM_CENTER_TO_SIDE);

    unsigned int AliveCount = 3;
    for (int i = 0; i < TARGET_COUNT; ++i)
    {
        D2D1_RECT_F targetRect = getImageRect(sTargetPosArr[i], OBJECT_DIST_FROM_CENTER_TO_SIDE);

        if (CheckCollision(targetRect, playerBulletRect))
        {
            sbAlive[i] = false;
            sPlayerBulletPos.y = -1.f;

            spDamageSoundPosition->put_CurrentPosition(0);
            spDamageSoundControl->Run();
        }

        if (!sbAlive[i])
        {
            sTargetPosArr[i].x = -1.f;
            sTargetPosArr[i].y = -1.f;

            --AliveCount;

            continue;
        }

        D2D1_RECT_F cheeseRect = getImageRect(sTargetBulletPosArr[i], BULLET_DIST_FROM_CENTER_TO_SIDE);

        if (CheckCollision(cheeseRect, playerRect))
        {
            sTargetBulletPosArr[i] = sTargetPosArr[i];
            sTargetBulletPosArr[i].y += BULLET_DIST_FROM_CENTER_TO_SIDE;

            spHitSoundPosition->put_CurrentPosition(0);
            spHitSoundControl->Run();

            continue;
        }

        if (sTargetBulletPosArr[i].y > SCREEN_HEIGHT)
        {
            sTargetBulletPosArr[i].y = sTargetPosArr[i].y + OBJECT_DIST_FROM_CENTER_TO_SIDE;
        }
        else
        {
            sTargetBulletPosArr[i].y += DELTA_TARGET_BULLET;
        }
    }

    Render();

    if (AliveCount == 0)
    {
        spawnTargets();
    }
}

static void GameManager::Render()
{
    assert(shWnd != nullptr);
    assert(sphWndRenderTarget != nullptr);
    assert(spPlayerBitmap != nullptr);
    assert(spPlayerBulletBitmap != nullptr);
    assert(spTargetBitmap != nullptr);
    assert(spPlayerBitmap != nullptr);

    const float DELTA_THETA = 1.f;
    static float theta = 0.f;
    theta += DELTA_THETA;

    sphWndRenderTarget->BeginDraw();
    {
        sphWndRenderTarget->Clear(
            D2D1::ColorF(D2D1::ColorF::White)
        );

        D2D1_RECT_F rt = getImageRect(sPlayerPos, OBJECT_DIST_FROM_CENTER_TO_SIDE);

        sphWndRenderTarget->DrawBitmap(
            spPlayerBitmap,
            rt
        );

        for (int i = 0; i < TARGET_COUNT; ++i)
        {
            if (!sbAlive[i])
            {
                continue;
            }

            rt = getImageRect(sTargetPosArr[i], OBJECT_DIST_FROM_CENTER_TO_SIDE);

            sphWndRenderTarget->DrawBitmap(
                spTargetBitmap,
                rt
            );

            sphWndRenderTarget->SetTransform(
                D2D1::Matrix3x2F::Rotation(theta, sTargetBulletPosArr[i])
            );

            sTargetBulletPosArr[i].y += DELTA_TARGET_BULLET;

            rt = getImageRect(sTargetBulletPosArr[i], BULLET_DIST_FROM_CENTER_TO_SIDE);

            sphWndRenderTarget->DrawBitmap(
                spTargetBulletBitmap,
                rt
            );

            sphWndRenderTarget->SetTransform(
                D2D1::Matrix3x2F::Identity()
            );
        }

        if (sPlayerBulletPos.y >= 0)
        {
            rt = getImageRect(sPlayerBulletPos, BULLET_DIST_FROM_CENTER_TO_SIDE);

            sphWndRenderTarget->DrawBitmap(
                spPlayerBulletBitmap,
                rt
            );
        }
    }
    sphWndRenderTarget->EndDraw();
}

inline D2D1_RECT_F getImageRect(const D2D1_POINT_2F pos, const float dist)
{
    D2D1_RECT_F rect;
    rect.left = pos.x - dist;
    rect.top = pos.y - dist;
    rect.right = pos.x + dist;
    rect.bottom = pos.y + dist;

    return rect;
}

void GameManager::OnKeyDown(const UINT keyCode)
{
    switch (keyCode)
    {
    case VK_LEFT:
        bLeftDown = true;
        break;

    case VK_RIGHT:
        bRightDown = true;
        break;

    case VK_SPACE:
        bSpaceDown = true;
        break;

    default:
        return;
    }
}

void GameManager::OnKeyUp(const UINT keyCode)
{
    switch (keyCode)
    {
    case VK_LEFT:
        bLeftDown = false;
        break;

    case VK_RIGHT:
        bRightDown = false;
        break;

    case VK_SPACE:
        bSpaceDown = false;
        break;

    default:
        return;
    }
}

static D2D1_POINT_2F generateTargetPos()
{
    float x = random(gen) * SCREEN_WIDTH;
    if (x + OBJECT_DIST_FROM_CENTER_TO_SIDE >= SCREEN_WIDTH)
    {
        x = SCREEN_WIDTH - OBJECT_DIST_FROM_CENTER_TO_SIDE;
    }
    else if (x - OBJECT_DIST_FROM_CENTER_TO_SIDE < 0.f)
    {
        x = OBJECT_DIST_FROM_CENTER_TO_SIDE;
    }

    const unsigned int HEIGHT_BOUNDRAY = SCREEN_HEIGHT / 3;

    float y = random(gen) * SCREEN_HEIGHT;
    if (y + OBJECT_DIST_FROM_CENTER_TO_SIDE > HEIGHT_BOUNDRAY)
    {
        y = HEIGHT_BOUNDRAY;
    }
    else if (y - OBJECT_DIST_FROM_CENTER_TO_SIDE < 0.f)
    {
        y = OBJECT_DIST_FROM_CENTER_TO_SIDE;
    }

    D2D1_POINT_2F pos;
    pos.x = x;
    pos.y = y;

    return pos;
}

static void spawnTargets()
{
    ZeroMemory(sTargetPosArr, sizeof(sTargetPosArr));

    for (int i = 0; i < TARGET_COUNT; ++i)
    {
        D2D1_POINT_2F position;
        while (true)
        {
            position = generateTargetPos();

            D2D1_RECT_F rect = getImageRect(position, OBJECT_DIST_FROM_CENTER_TO_SIDE);

            for (const D2D1_POINT_2F pos : sTargetPosArr)
            {
                D2D1_RECT_F targetRect = getImageRect(pos, OBJECT_DIST_FROM_CENTER_TO_SIDE);

                if (CheckCollision(rect, targetRect))
                {
                    goto COLLISION;
                }
            }

            break;

        COLLISION:;
        }

        sTargetPosArr[i] = position;
        sTargetBulletPosArr[i] = {
            position.x,
            position.y = position.y + OBJECT_DIST_FROM_CENTER_TO_SIDE
        };

        sbAlive[i] = true;
    }
}

static bool CheckCollision(const D2D1_RECT_F rect1, const D2D1_RECT_F rect2)
{
    if (rect1.left > rect2.right || rect1.right < rect2.left)
    {
        return false;
    }

    if (rect1.top > rect2.bottom || rect1.bottom < rect2.top)
    {
        return false;
    }

    return true;
}

#include "GameManager.h"

static HWND shWnd = nullptr;

static ID2D1HwndRenderTarget* sphWndRenderTarget = nullptr;

static std::random_device sRandomDevice;
static std::mt19937_64 gen(sRandomDevice());
static std::uniform_real_distribution<float> random(0.f, 1.f);

enum
{
    TARGET_COUNT = 3
};

static Player* spPlayer = nullptr;

static std::vector<Object*> sObjectPtrs;
static std::vector<IMovable*> sMovableObjectPtrs;
static std::vector<AttackableObject*> sAttackableObjectPtrs;

static std::vector<Target*> sTargetPtrs;

void loadImage(const TCHAR* filePath, ID2D1Bitmap** outBitmapPtr);
static D2D1_POINT_2F generateTargetPos();
static void spawnTargets();
static bool CheckCollision(const D2D1_RECT_F rect1, const D2D1_RECT_F rect2);

void GameManager::Initialize(const HWND hWnd)
{
    shWnd = hWnd;

    // (표적의 수 + 표적의 총알 수) + (플레이어의 수 + 플레이어 총알의 수);
    sObjectPtrs.reserve((TARGET_COUNT + TARGET_COUNT) + (1 + 1));

    // 플레이어, 총알의 총 개수
    sMovableObjectPtrs.reserve((1 + 1) + TARGET_COUNT);

    // 플레이어 + 표적의 수
    sAttackableObjectPtrs.reserve(1 + TARGET_COUNT);

    sTargetPtrs.reserve(TARGET_COUNT);

    ID2D1Factory* pD2DFactory = nullptr;
    {
        ID2D1Bitmap* pPlayerBitmap = nullptr;
        ID2D1Bitmap* pPlayerBulletBitmap = nullptr;

        ID2D1Bitmap* pTargetBitmap = nullptr;
        ID2D1Bitmap* pTargetBulletBitmap = nullptr;

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

        loadImage(TEXT("./Data/Image/Bini.jpg"), &pPlayerBitmap);
        loadImage(TEXT("./Data/Image/Heart.png"), &pPlayerBulletBitmap);

        loadImage(TEXT("./Data/Image/Bokdang.jpg"), &pTargetBitmap);
        loadImage(TEXT("./Data/Image/Cheese.png"), &pTargetBulletBitmap);

        const float DELTA_PLAYER_BULLET = 10.f;

        Bullet* pPlayerBullet = new Bullet(pPlayerBulletBitmap, -DELTA_PLAYER_BULLET);

        sObjectPtrs.push_back(pPlayerBullet);
        sMovableObjectPtrs.push_back(pPlayerBullet);

        spPlayer = new Player(pPlayerBitmap, pPlayerBullet);

        sAttackableObjectPtrs.push_back(spPlayer);
        sMovableObjectPtrs.push_back(spPlayer);

        const float DELTA_TARGET_BULLET = 2.f;

        for (int i = 0; i < TARGET_COUNT; ++i)
        {
            pTargetBulletBitmap->AddRef();
            Bullet* pTargetBullet = new Bullet(pTargetBulletBitmap, DELTA_TARGET_BULLET);

            sObjectPtrs.push_back(pTargetBullet);
            sMovableObjectPtrs.push_back(pTargetBullet);

            pTargetBitmap->AddRef();
            Target* pTarget = new Target(pTargetBitmap, pTargetBullet);

            sTargetPtrs.push_back(pTarget);
            sAttackableObjectPtrs.push_back(pTarget);
        }
        // 초과 카운팅 제거
        COM_RELEASE(pTargetBulletBitmap);
        COM_RELEASE(pTargetBitmap);

        // 그리기 순서 조정을 위해서 마지막에 대입
        sObjectPtrs.push_back(spPlayer);
        for (Target* t : sTargetPtrs)
        {
            sObjectPtrs.push_back(t);
        }
    }
    COM_RELEASE(pD2DFactory);

    spawnTargets();
}

void GameManager::Destroy()
{
    CoUninitialize();

    delete spPlayer;

    for (Target* t : sTargetPtrs)
    {
        delete t;
    }

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

void GameManager::Run()
{
    for (IMovable* m : sMovableObjectPtrs)
    {
        m->Move();
    }

    for (AttackableObject* ao : sAttackableObjectPtrs)
    {
        ao->Attack();
    }

    unsigned int AliveCount = TARGET_COUNT;

    // 충돌 확인
    D2D1_RECT_F playerRect = spPlayer->GetCollisionRect();

    Bullet* pPlayerBullet = spPlayer->GetBullet();
    D2D1_RECT_F playerBulletRect = pPlayerBullet->GetCollisionRect();

    for (int i = 0; i < TARGET_COUNT; ++i)
    {
        Target* pTarget = sTargetPtrs[i];

        Bullet* pTargetBullet = pTarget->GetBullet();
        D2D1_RECT_F targetBulletRect = pTargetBullet->GetCollisionRect();

        if (CheckCollision(playerRect, targetBulletRect))
        {
            spPlayer->OnCollision();
            pTargetBullet->OnCollision();
        }

        if (pTarget->IsDead())
        {
            --AliveCount;
            continue;
        }

        D2D1_RECT_F targetRect = pTarget->GetCollisionRect();

        if (CheckCollision(targetRect, playerBulletRect))
        {
            pTarget->OnCollision();
            pPlayerBullet->OnCollision();

            --AliveCount;
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

    const float DELTA_THETA = 1.f;
    static float theta = 0.f;
    theta += DELTA_THETA;

    sphWndRenderTarget->BeginDraw();
    {
        sphWndRenderTarget->Clear(
            D2D1::ColorF(D2D1::ColorF::White)
        );

        for (Object* o : sObjectPtrs)
        {
            o->Draw(*sphWndRenderTarget);
        }
    }
    sphWndRenderTarget->EndDraw();
}

void GameManager::OnKeyDown(const UINT keyCode)
{
    switch (keyCode)
    {
    case VK_LEFT:
        spPlayer->bLeftDown = true;
        break;

    case VK_RIGHT:
        spPlayer->bRightDown = true;
        break;

    case VK_SPACE:
        spPlayer->bSpaceDown = true;
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
        spPlayer->bLeftDown = false;
        break;

    case VK_RIGHT:
        spPlayer->bRightDown = false;
        break;

    case VK_SPACE:
        spPlayer->bSpaceDown = false;
        break;

    default:
        return;
    }
}

static D2D1_POINT_2F generateTargetPos()
{
    float x = random(gen) * SCREEN_WIDTH;
    if (x + TARGET_DIST_FROM_CENTER_TO_SIDE >= SCREEN_WIDTH)
    {
        x = SCREEN_WIDTH - TARGET_DIST_FROM_CENTER_TO_SIDE;
    }
    else if (x - TARGET_DIST_FROM_CENTER_TO_SIDE < 0.f)
    {
        x = TARGET_DIST_FROM_CENTER_TO_SIDE;
    }

    const unsigned int HEIGHT_BOUNDRAY = SCREEN_HEIGHT / 3;

    float y = random(gen) * SCREEN_HEIGHT;
    if (y + TARGET_DIST_FROM_CENTER_TO_SIDE > HEIGHT_BOUNDRAY)
    {
        y = HEIGHT_BOUNDRAY;
    }
    else if (y - TARGET_DIST_FROM_CENTER_TO_SIDE < 0.f)
    {
        y = TARGET_DIST_FROM_CENTER_TO_SIDE;
    }

    D2D1_POINT_2F pos;
    pos.x = x;
    pos.y = y;

    return pos;
}

static void spawnTargets()
{
    for (Target* t : sTargetPtrs)
    {
        t->ResetPosition();

        D2D1_POINT_2F position;
        while (true)
        {
            position = generateTargetPos();

            D2D1_RECT_F rect = GetImageRect(position, TARGET_DIST_FROM_CENTER_TO_SIDE);

            for (Target* t : sTargetPtrs)
            {
                D2D1_RECT_F targetRect = t->GetCollisionRect();;

                if (CheckCollision(rect, targetRect))
                {
                    goto COLLISION;
                }
            }

            break;

        COLLISION:;
        }

        t->Revive(position);
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

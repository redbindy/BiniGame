#include "AudioController.h"

AudioController::AudioController(const TCHAR* filePath)
    : mpGraph(nullptr)
    , mpControl(nullptr)
    , mpPosition(nullptr)
    , mpAudio(nullptr)
{
    HRESULT hr = CoCreateInstance(
        CLSID_FilterGraph,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_IGraphBuilder,
        (void**)&mpGraph
    );
    assert(SUCCEEDED(hr));

    hr = mpGraph->QueryInterface(
        IID_IMediaControl,
        (void**)&mpControl
    );
    assert(SUCCEEDED(hr));

    hr = mpGraph->QueryInterface(
        IID_IMediaPosition,
        (void**)&mpPosition
    );
    assert(SUCCEEDED(hr));

    hr = mpGraph->RenderFile(filePath, nullptr);
    assert(SUCCEEDED(hr));
}

AudioController::~AudioController()
{
    COM_RELEASE(mpAudio);
    COM_RELEASE(mpPosition);
    COM_RELEASE(mpControl);
    COM_RELEASE(mpGraph);
}

void AudioController::TurnVolumeDown(const long minusVolume)
{
    assert(minusVolume <= 0);

    HRESULT hr;
    if (mpAudio == nullptr)
    {
        hr = mpGraph->QueryInterface(
            IID_IBasicAudio,
            (void**)&mpAudio
        );
        assert(SUCCEEDED(hr));
    }

    hr = mpAudio->put_Volume(minusVolume);
    assert(SUCCEEDED(hr));
}

void AudioController::PlayAudio()
{
    HRESULT hr = mpPosition->put_CurrentPosition(0);
    assert(SUCCEEDED(hr));

    hr = mpControl->Run();
    assert(SUCCEEDED(hr));
}

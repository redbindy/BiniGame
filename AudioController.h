#pragma once

#include <dshow.h>
#pragma comment(lib, "strmiids.lib")

#include "framework.h"
#include "ComHelper.h"

class AudioController final
{
public:
    AudioController(const TCHAR* filePath);
    ~AudioController();
    AudioController(const AudioController& other) = default;
    AudioController& operator=(const AudioController& other) = default;

    void TurnVolumeDown(const long minusVolume);
    void PlayAudio();

private:
    IGraphBuilder* mpGraph;
    IMediaControl* mpControl;
    IMediaPosition* mpPosition;
    IBasicAudio* mpAudio;
};
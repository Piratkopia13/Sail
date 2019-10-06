//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

// TODO: include link to where the original code is from

#pragma once

//#include "AudioFileReader.h"
//#include "..\AudioEngine\globals.hpp"


//#include <x3daudio.h>
#include <string>
#include <stack>



// TODO: remove unneeded stuff
#include <XAudio2.h>
#include <xapo.h>
#include <hrtfapoapi.h>
#include <DirectXMath.h>
#include <wrl\client.h>
#include <wrl\implements.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <strsafe.h>

#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>

#pragma comment(lib, "Mfreadwrite.lib")
#pragma comment(lib, "Mfplat.lib")
#pragma comment(lib, "Mfuuid.lib")
#pragma comment(lib, "hrtfapo.lib")


using namespace Microsoft::WRL;
//using namespace Windows::UI::Xaml;
using namespace Windows::Foundation;

//#include "AudioFileReader.h"
//#include "XAudio2Helpers.h"
//#include "App.xaml.h"
//#include "OmnidirectionalSound.h"
//#include "CardioidSound.h"
//#include "CustomDecay.h"

#include "API/Audio/AudioFileReader.h"
//#include "API/Audio/WaveBankReader.h" // TODO: Use this instead/too?
#include "API/Audio/XAudio2Helpers.h"

//
// Sound with omnidirectional radiation pattern i.e. emits sound equally in all directions.
//
class OmnidirectionalSound
{
public:
	OmnidirectionalSound(const std::string& path);

	virtual ~OmnidirectionalSound();
	//HRESULT Initialize(LPCWSTR filename);
	HRESULT Initialize();

	HRESULT Start();
	HRESULT Stop();
	//HRESULT OnUpdate(const glm::vec3& pos);
	//HRESULT SetEnvironment(HrtfEnvironment environment);
	HrtfEnvironment GetEnvironment() { return _environment; }

//private:
	//HrtfPosition ComputePositionInOrbit(float height, _In_ float radius, _In_ float angle);

//private:
	std::string                 _filename = "";
	bool                        _isQueued = false;
	bool                        _isPlaying = false;
	AudioFileReader             _audioFile;
	ComPtr<IXAudio2>            _xaudio2;
	IXAudio2SourceVoice*        _sourceVoice = nullptr;
	ComPtr<IXAPOHrtfParameters> _hrtfParams;
	HrtfEnvironment             _environment = HrtfEnvironment::Outdoors;
	ULONGLONG                   _lastTick = 0;
	float                       _angle = 0;
};

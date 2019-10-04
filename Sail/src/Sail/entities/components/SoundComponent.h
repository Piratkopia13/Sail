#pragma once

#include "Sail/entities/components/Component.h"

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
//namespace SoundType {
//	enum SoundType { WALK, RUN, SHOOT, JUMP, LANDING, COUNT };
//}

class SoundComponent : public Component<SoundComponent>
{
public:
	SoundComponent() {}
	virtual ~SoundComponent() {
		if (_sourceVoice) {
			_sourceVoice->DestroyVoice();
		}
	}

	//X3DAUDIO_LISTENER listener;

	//std::string m_soundEffects[SoundType::COUNT];
	//int m_soundID[SoundType::COUNT];
	//float m_soundEffectTimers[SoundType::COUNT];
	//float m_soundEffectThresholds[SoundType::COUNT];
	//bool m_isPlaying[SoundType::COUNT];
	//bool m_playOnce[SoundType::COUNT];

	//// • string = filename
	//// • bool = TRUE if START-request, FALSE if STOP-request
	//std::list<std::pair<std::string, bool>> m_streamingRequests;
	//// • string = filename
	//// • int = ID of playing streaming; needed for STOPPING the streamed sound
	//std::list<std::pair<std::string, int>> m_currentlyStreaming;

	//// This function is purely here to MAKE LIFE LESS DIFFICULT
	//void defineSound(SoundType::SoundType type, std::string filename, float dtThreshold = 0.0f, bool playOnce = true);

	


	// TODO: allow sound components to emit multiple sounds simultaneously
	
	bool soundQueued = false;
	bool soundPlaying = false;
	std::string filename = ""; // TODO? save as LPCWSTR
	//LPCWSTR filename = L"";

	// TODO: sound position, either sticking with the entity's transform or remaining stationary
	
	AudioFileReader                 _audioFile;
	ComPtr<IXAudio2>                _xaudio2;
	IXAudio2SourceVoice*            _sourceVoice = nullptr;
	ComPtr<IXAPOHrtfParameters>     _hrtfParams;
	HrtfEnvironment                 _environment = HrtfEnvironment::Outdoors;
	ULONGLONG                       _lastTick = 0;
	float                           _angle = 0;




};


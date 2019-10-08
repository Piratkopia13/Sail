#include "pch.h"
#include "HRTFAudioSystem.h"

#include "Sail/graphics/camera/Camera.h"

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


//using namespace Microsoft::WRL;
//using namespace Windows::UI::Xaml;
//using namespace Windows::Foundation;

//#include "AudioFileReader.h"
//#include "XAudio2Helpers.h"
//#include "App.xaml.h"
//#include "OmnidirectionalSound.h"
//#include "CardioidSound.h"
//#include "CustomDecay.h"

#include "API/Audio/AudioFileReader.h"
//#include "API/Audio/WaveBankReader.h" // TODO: Use this instead/too?
#include "API/Audio/XAudio2Helpers.h"

//#include "XAudio2Engine\OmnidirectionalSound.h"
#include "Sail/entities/Entity.h"
#include "Sail/entities/ECS.h"
#include "Sail/entities/components/SoundComponent.h"
#include "Sail/entities/components/TransformComponent.h"

#define HRTF_2PI    6.283185307f


HRTFAudioSystem::HRTFAudioSystem() : BaseComponentSystem()
{
	registerComponent<SoundComponent>(true, true, true);
	registerComponent<TransformComponent>(true, true, true);
}

HRTFAudioSystem::~HRTFAudioSystem()
{

}

void HRTFAudioSystem::update(float dt)
{
	//for (auto e : entities) {
	//	//auto sc = e->getComponent<SoundComponent>();

	//	for (auto& sound : e->getComponent<SoundComponent>()->sounds) {
	//		if (sound->_isPlaying || sound->_isQueued) {
	//			updateSoundWithNewPosition(*sound);
	//		}
	//		if (sound->_isQueued) {
	//			sound->Start();
	//			sound->_isPlaying = true;
	//			sound->_isQueued = false;
	//		}
	//	}

	//	//// If the component has had a sound queued initialize that sound
	//	//if (sc->soundQueued) {
	//	//	initializeSound(*sc);
	//	//	sc->soundQueued = false;
	//	//	sc->soundPlaying = true;
	//	//}

	//	//// If the component is currently playing a sound update it with a new position
	//	//if (sc->soundPlaying) {
	//	//	updateSoundWithNewPosition(*sc);
	//	//}
	//}
}

void HRTFAudioSystem::update(Camera& cam, float alpha) const {
	for (auto e : entities) {
		//auto sc = e->getComponent<SoundComponent>();

		for (auto& sound : e->getComponent<SoundComponent>()->sounds) {
			if (sound->_isPlaying || sound->_isQueued) {
				//auto* transform = e->getComponent<TransformComponent>();
				updateSoundWithNewPosition(*sound, cam, *e->getComponent<TransformComponent>(), alpha);
			}
			if (sound->_isQueued) {
				sound->Start();
				sound->_isPlaying = true;
				sound->_isQueued = false;
			}
		}

		//// If the component has had a sound queued initialize that sound
		//if (sc->soundQueued) {
		//	initializeSound(*sc);
		//	sc->soundQueued = false;
		//	sc->soundPlaying = true;
		//}

		//// If the component is currently playing a sound update it with a new position
		//if (sc->soundPlaying) {
		//	updateSoundWithNewPosition(*sc);
		//}
	}
}

void HRTFAudioSystem::initializeSound(OmnidirectionalSound& sound) const {
	// conversion from std::string to LPCWSTR (Long Pointer to Const Wide STRing)
	int stringLength = MultiByteToWideChar(CP_ACP, 0, sound._filename.data(), sound._filename.length(), 0, 0);
	std::wstring wstr(stringLength, 0);
	MultiByteToWideChar(CP_ACP, 0, sound._filename.data(), sound._filename.length(), &wstr[0], stringLength);


	auto hr = sound._audioFile.Initialize(wstr.c_str());

	ComPtr<IXAPO> xapo;
	if (SUCCEEDED(hr))
	{
		// Passing in nullptr as the first arg for HrtfApoInit initializes the APO with defaults of
		// omnidirectional sound with natural distance decay behavior.
		// CreateHrtfApo will fail with E_NOTIMPL on unsupported platforms.
		hr = CreateHrtfApo(nullptr, &xapo);
	}

	if (SUCCEEDED(hr))
	{
		hr = xapo.As(&sound._hrtfParams);
	}

	// Set the default environment.
	if (SUCCEEDED(hr))
	{
		hr = sound._hrtfParams->SetEnvironment(sound._environment);
	}


	// Initialize an XAudio2 graph that hosts the HRTF xAPO.
	// The source voice is used to submit audio data and control playback.
	if (SUCCEEDED(hr))
	{
		hr = SetupXAudio2(sound._audioFile.GetFormat(), xapo.Get(), &sound._xaudio2, &sound._sourceVoice);
	}

	// Submit audio data to the source voice.
	if (SUCCEEDED(hr))
	{
		XAUDIO2_BUFFER buffer{};
		buffer.AudioBytes = static_cast<UINT32>(sound._audioFile.GetSize());
		buffer.pAudioData = sound._audioFile.GetData();
		buffer.LoopCount = XAUDIO2_LOOP_INFINITE;
		hr = sound._sourceVoice->SubmitSourceBuffer(&buffer);
	}
}

// TODO: position offset stuff
void HRTFAudioSystem::updateSoundWithNewPosition(OmnidirectionalSound& sound, Camera& cam, TransformComponent& transform, float alpha) const {
	// Sound source position currently hard coded
	// TODO: use the position from the entities transform component,
	// perhaps with a per-sound offset if you want footsteps to come from the feet for example
	//glm::vec4 soundPosition = { 1.6 - 10.f,1.8,10.0f,1 };


	// soundPos = transformPos + offset*rotation
	glm::vec3 soundPos = transform.getInterpolatedTranslation(alpha);

	// if (offset != 0) { rotate offset }
	//if ()


	glm::vec3 negativeZAxis = glm::normalize(cam.getDirection());
	glm::vec3 positiveYAxisGuess = glm::normalize(-cam.getUp());
	glm::vec3 positiveXAxis = glm::normalize(glm::cross(negativeZAxis, positiveYAxisGuess));
	glm::vec3 positiveYAxis = glm::normalize(cross(negativeZAxis, positiveXAxis));


	glm::mat4x4 rotationTransform{
		positiveXAxis.x, positiveYAxis.x, negativeZAxis.x, 0.f,
		positiveXAxis.y, positiveYAxis.y, negativeZAxis.y, 0.f,
		positiveXAxis.z, positiveYAxis.z, negativeZAxis.z, 0.f,
		0.f, 0.f, 0.f, 1.f,
	};

	// Reinterpret the sound's position in the device's coordinate system.
	glm::vec3 soundRelativeToHead = glm::vec3((rotationTransform * glm::translate(-cam.getPosition())) * glm::vec4(soundPos, 1.f));

	// Note that at (0, 0, 0) exactly, the HRTF audio will simply pass through audio. We can use a minimal offset
	// to simulate a zero distance when the hologram position vector is exactly at the device origin in order to
	// allow HRTF to continue functioning in this edge case.
	float distanceFromHologramToHead = glm::length(soundRelativeToHead);

	Logger::Log("X: " + std::to_string(soundRelativeToHead.x) 
		+ " Y: " + std::to_string(soundRelativeToHead.y) +
		+ " Z: " + std::to_string(soundRelativeToHead.z) );

	static const float distanceMin = 0.00001f;
	if (distanceFromHologramToHead < distanceMin) {
		soundRelativeToHead = glm::vec3(0.f, distanceMin, 0.f);
	}

	auto hrtfPosition = HrtfPosition{
		soundRelativeToHead.x,
		soundRelativeToHead.y,
		soundRelativeToHead.z
	};

	// update the source position with the new relative position
	sound._hrtfParams->SetSourcePosition(&hrtfPosition);
}

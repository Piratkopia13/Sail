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
#include "pch.h"

#include "OmnidirectionalSound.h"

//#include "Sound.h"
#include <string>


OmnidirectionalSound::OmnidirectionalSound(const std::string& path) : _filename(path)
{
	/*int stringLength = MultiByteToWideChar(CP_ACP, 0, path.data(), path.length(), 0, 0);
	std::wstring wstr(stringLength, 0);
	MultiByteToWideChar(CP_ACP, 0, path.data(), path.length(), &wstr[0], stringLength);

	Initialize(wstr.c_str());

	if (FAILED(Initialize(wstr.c_str()))) {
		int a = 43;
	}*/
}

_Use_decl_annotations_
//HRESULT OmnidirectionalSound::Initialize(LPCWSTR filename)
HRESULT OmnidirectionalSound::Initialize()
{
	// TODO: See if this can be done more cleanly
	int stringLength = MultiByteToWideChar(CP_ACP, 0, _filename.data(), _filename.length(), 0, 0);
	std::wstring wstr(stringLength, 0);
	MultiByteToWideChar(CP_ACP, 0, _filename.data(), _filename.length(), &wstr[0], stringLength);

	//Initialize(wstr.c_str());


	auto hr = _audioFile.Initialize(wstr.c_str());

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
		hr = xapo.As(&_hrtfParams);
	}

	// Set the default environment.
	if (SUCCEEDED(hr))
	{
		hr = _hrtfParams->SetEnvironment(_environment);
	}

	// Initialize an XAudio2 graph that hosts the HRTF xAPO.
	// The source voice is used to submit audio data and control playback.
	if (SUCCEEDED(hr))
	{
		hr = SetupXAudio2(_audioFile.GetFormat(), xapo.Get(), &_xaudio2, &_sourceVoice);
	}

	// Submit audio data to the source voice.
	if (SUCCEEDED(hr))
	{
		XAUDIO2_BUFFER buffer{};
		buffer.AudioBytes = static_cast<UINT32>(_audioFile.GetSize());
		buffer.pAudioData = _audioFile.GetData();
		buffer.LoopCount = XAUDIO2_LOOP_INFINITE;
		hr = _sourceVoice->SubmitSourceBuffer(&buffer);
	}

	return hr;
}

OmnidirectionalSound::~OmnidirectionalSound()
{
	if (_sourceVoice)
	{
		_sourceVoice->DestroyVoice();
	}
}

HRESULT OmnidirectionalSound::Start()
{
	_lastTick = GetTickCount64();
	return _sourceVoice->Start();
}

HRESULT OmnidirectionalSound::Stop()
{
	return _sourceVoice->Stop();
}

//_Use_decl_annotations_
//HRESULT OmnidirectionalSound::SetEnvironment(HrtfEnvironment environment)
//{
//	// Environment can be changed at any time.
//	return _hrtfParams->SetEnvironment(environment);
//}

//
// This method is called on every dispatcher timer tick.
// Caller passes in the information necessary to compute the source position.
//
//_Use_decl_annotations_
//HRESULT OmnidirectionalSound::OnUpdate(const glm::vec3& pos)
//{
//	auto hrtfPosition = HrtfPosition{ pos.x, pos.y, pos.z };
//	return _hrtfParams->SetSourcePosition(&hrtfPosition);
//
//	/*auto tick = GetTickCount64();
//	auto elapsedTime = tick - _lastTick;
//	_lastTick = tick;
//	_angle += elapsedTime * angularVelocity;
//	_angle = _angle > HRTF_2PI ? (_angle - HRTF_2PI) : _angle;
//	auto position = ComputePositionInOrbit(height, radius, _angle);
//	return _hrtfParams->SetSourcePosition(&position);*/
//}

//_Use_decl_annotations_
//HrtfPosition OmnidirectionalSound::ComputePositionInOrbit(float height, float radius, float angle)
//{
//	// Calculate the position of the source based on height relative to listerner's head, radius of orbit and angle relative to the listener.
//	// APO uses right-handed coordinate system where -ve z-axis is forward and +ve z-axis is backward.
//	// All coordinates use real-world units (meters).
//	float x = radius * sin(angle);
//	float z = -radius * cos(angle);
//	return HrtfPosition{ x, height ,z };
//}


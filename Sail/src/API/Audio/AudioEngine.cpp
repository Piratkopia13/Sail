#include "pch.h"
#include "API/Audio/AudioEngine.h"
#include "MemoryManager/MemoryManager/src/MemoryManager.h"
#include "Sail/Application.h"
#include "Sail/KeyBinds.h"
#include "API/Audio/GeneralFunctions.h"
#include <fstream>
#include "WaveBankReader.h"
#include <math.h>


#include "Sail/graphics/geometry/Transform.h"
#include "Sail/graphics/camera/Camera.h"

AudioEngine::AudioEngine() {

	HRESULT hr;
	hr = CoInitialize(nullptr);

#pragma region ERROR_CHECKING
	try {
		if (hr != S_OK) {
			throw std::invalid_argument(nullptr);
		}
	}
	catch (const std::invalid_argument& e) {
		UNREFERENCED_PARAMETER(e);
		wchar_t errorMsgBuffer[256];
		wsprintfW(errorMsgBuffer, L"FUNCTION: Audio::Audio()\n\nMESSAGE: The 'CoInitialize' function failed!");
		MessageBox(NULL, errorMsgBuffer, static_cast<LPCWSTR>(L"AUDIO ERROR!"), MB_ICONERROR);
		std::exit(0);
	}
#pragma endregion

	this->initialize();
}

AudioEngine::~AudioEngine() {
	m_isRunning = false;

	//delete m_DSPSettings.pMatrixCoefficients;
	//delete m_DSPSettings.pDelayTimes;
}

void AudioEngine::loadSound(const std::string& filename) {
	Application::getInstance()->getResourceManager().loadAudioData(filename, m_xAudio2);
}

int AudioEngine::initializeSound(const std::string& filename) {
	if (m_masterVoice == nullptr) {
		Logger::Error("'IXAudio2MasterVoice' has not been correctly initialized; audio is unplayable!");
		return -1;
	}
	if (Application::getInstance()->getResourceManager().hasAudioData(filename)) {
		int indexValue = m_currSoundIndex; // Store early
		m_currSoundIndex++;
		m_currSoundIndex %= SOUND_COUNT;

		if (m_sound[indexValue].sourceVoice != nullptr) {
			m_sound[indexValue].sourceVoice->Stop();
			m_sound[indexValue].sourceVoice->DestroyVoice();
		}


		// HRTF stuff

		Microsoft::WRL::ComPtr<IXAPO> xapo;
		// Passing in nullptr as the first arg for HrtfApoInit initializes the APO with defaults of
		// omnidirectional sound with natural distance decay behavior.
		// CreateHrtfApo will fail with E_NOTIMPL on unsupported platforms.
		HRESULT hr = CreateHrtfApo(nullptr, &xapo);

		if (SUCCEEDED(hr))
		{
			hr = xapo.As(&m_sound[indexValue].hrtfParams);
		}

		// Set the default environment.
		if (SUCCEEDED(hr))
		{
			hr = m_sound[indexValue].hrtfParams->SetEnvironment(m_sound[indexValue].environment);
		}

		// creating a 'sourceVoice' for WAV file-type
		//HRESULT hr = m_xAudio2->CreateSourceVoice(&m_sound[indexValue].sourceVoice, (WAVEFORMATEX*)Application::getInstance()->getResourceManager().getAudioData(filename).getFormat());
		if (SUCCEEDED(hr)) {
			hr = m_xAudio2->CreateSourceVoice(&m_sound[indexValue].sourceVoice, (WAVEFORMATEX*)Application::getInstance()->getResourceManager().getAudioData(filename).getFormat());
		}

		// THIS IS THE OTHER VERSION FOR ADPC
				// ... for ADPC-WAV compressed file-type
				//hr = xAudio->CreateSourceVoice(&pSourceVoice, (WAVEFORMATEX*)& adpcwf);

		if (FAILED(hr)) {
			Logger::Error("Failed to create the actual 'SourceVoice' for a sound file!");
			return -1;
		}


		// Create a submix voice that will host the xAPO.
		// This submix voice will be destroyed when XAudio2 instance is destroyed.
		IXAudio2SubmixVoice* submixVoice = nullptr;

		if (SUCCEEDED(hr))
		{
			XAUDIO2_EFFECT_DESCRIPTOR fxDesc{};
			fxDesc.InitialState = TRUE;
			fxDesc.OutputChannels = 2;          // Stereo output
			fxDesc.pEffect = xapo.Get();        // HRTF xAPO set as the effect.

			XAUDIO2_EFFECT_CHAIN fxChain{};
			fxChain.EffectCount = 1;
			fxChain.pEffectDescriptors = &fxDesc;

			XAUDIO2_VOICE_SENDS sends = {};
			XAUDIO2_SEND_DESCRIPTOR sendDesc = {};
			sendDesc.pOutputVoice = m_masterVoice;
			sends.SendCount = 1;
			sends.pSends = &sendDesc;

			// HRTF APO expects mono 48kHz input, so we configure the submix voice for that format.
			hr = m_xAudio2->CreateSubmixVoice(&submixVoice, 1, 48000, 0, 0, &sends, &fxChain);
		}

		// Route the source voice to the submix voice.
		// The complete graph pipeline looks like this -
		// Source Voice -> Submix Voice (HRTF xAPO) -> Mastering Voice
		if (SUCCEEDED(hr))
		{
			XAUDIO2_VOICE_SENDS sends = {};
			XAUDIO2_SEND_DESCRIPTOR sendDesc = {};
			sendDesc.pOutputVoice = submixVoice;
			sends.SendCount = 1;
			sends.pSends = &sendDesc;
			hr = m_sound[indexValue].sourceVoice->SetOutputVoices(&sends);
		}

		// Submit audio data to the source voice.
		if (SUCCEEDED(hr)) {
			hr = m_sound[indexValue].sourceVoice->SubmitSourceBuffer(Application::getInstance()->getResourceManager().getAudioData(filename).getSoundBuffer());
		}

		if (FAILED(hr)) {
			Logger::Error("Failed to submit the 'sourceBuffer' to the 'sourceVoice' for a sound file!");
			return -1;
		}
		{
			//m_sound[indexValue].sourceVoice->SetVolume(VOL_THIRD);

			//m_sound[indexValue].emitter.OrientFront = listener.OrientFront;
			//m_sound[indexValue].emitter.OrientTop = listener.OrientTop;
			//m_sound[indexValue].emitter.Position = { listener.Position.x, listener.Position.y, listener.Position.z };
			//m_sound[indexValue].emitter.Velocity = listener.Velocity;

			//m_tempDistance += 0.1f;

			//X3DAudioCalculate(m_X3DInstance, &listener, &m_sound[indexValue].emitter,
			//	X3DAUDIO_CALCULATE_MATRIX | X3DAUDIO_CALCULATE_DOPPLER | X3DAUDIO_CALCULATE_LPF_DIRECT /*| X3DAUDIO_CALCULATE_REVERB*/,
			//	&m_DSPSettings);

			//IXAudio2Voice* pDestb4 = nullptr;
			//float* pMatrixb4 = new float[64];
			//m_sound[indexValue].sourceVoice->GetOutputMatrix(pDest, 1, 2, pMatrix);
			//delete[]pMatrixb4;

			//XAUDIO2_FILTER_PARAMETERS FilterParameters = {
			//	LowPassFilter,
			//	/*2 * sin(pi * (desired filter cutoff frequency) / sampleRate) */
			//	2.0f * sinf(X3DAUDIO_PI * 20000 / 48000), //6.0f * m_DSPSettings.LPFDirectCoefficient
			//	1.0f
			//};

			//hr = m_sound[indexValue].sourceVoice->SetOutputMatrix(m_masterVoice, 2, m_destinationChannelCount, m_DSPSettings.pMatrixCoefficients);
			//if (hr != S_OK) {
			//	Logger::Error("UN-NAMED ERROR3!");
			//	return -1;
			//}

			//if (m_xAudio2->CommitChanges(XAUDIO2_COMMIT_ALL) == S_OK) {
			//	int asdf = 3;
			//} else {
			//	int asdf = 3;
			//}

			//float* pMatrix = new float[2];
			//m_sound[indexValue].sourceVoice->GetOutputMatrix(NULL, 2, 2, pMatrix);
			//delete[]pMatrix;
			//m_sound[indexValue].sourceVoice->SetFrequencyRatio(m_DSPSettings.DopplerFactor);

			// R E V E R B   A P P L I C A T I O N
			//m_sound[returnValue].sourceVoice->SetOutputMatrix(m_masterVoice, 1, m_destinationChannelCount, &DSPSettings.ReverbLevel);
			// EXAMPLE-CODE VERSION: pSFXSourceVoice->SetOutputMatrix(pSubmixVoice, 1, 1, &DSPSettings.ReverbLevel);



			//hr = m_xAudio2->CommitChanges(XAUDIO2_COMMIT_ALL);
			//if (hr != S_OK) {
			//	Logger::Error("UN-NAMED ERROR5!");
			//	return -1;
			//}
			/*XAUDIO2_FILTER_PARAMETERS work{
				LowPassFilter,
				1.0f,
				1.0f,
			};*/
			// IFAN, USE THE SOURCE (it's broken)
			//hr = m_sound[indexValue].sourceVoice->SetFilterParameters(&work/*, XAUDIO2_COMMIT_NOW*/);
			//if (hr != S_OK) {
			//	Logger::Error("UN-NAMED ERROR4!");
			//	return -1;
			//}

			//hr = m_sound[indexValue].sourceVoice->Start(0, XAUDIO2_COMMIT_ALL);
			//if (m_xAudio2->CommitChanges(XAUDIO2_COMMIT_ALL) == S_OK) {
			//	int asdf = 3;
			//}

			//if (hr != S_OK) {
			//	Logger::Error("Failed submit processed audio data to data buffer for a audio file");
			//	return -1;
			//}

			//std::cout << "LEFT_EAR_VOL: " << m_DSPSettings.pMatrixCoefficients[0] << "\n";
			//std::cout << "RIGHT_EAR_VOL: " << m_DSPSettings.pMatrixCoefficients[1] << "\n";
			//std::cout << "DISTANCE: " << m_DSPSettings.EmitterToListenerDistance << "\n";
			//float tempFloat;
			//m_sound[indexValue].sourceVoice->GetVolume(&tempFloat);
			//std::cout << "CURRENT VOLUME: " << tempFloat << "\n";
		}
		return indexValue;


		{

			//int indexValue = m_currSoundIndex; // Store early
			//m_currSoundIndex++;
			//m_currSoundIndex %= SOUND_COUNT;

			//if (m_sound[indexValue].sourceVoice != nullptr) {
			//	m_sound[indexValue].sourceVoice->Stop();
			//	m_sound[indexValue].sourceVoice->DestroyVoice();
			//}

			//// creating a 'sourceVoice' for WAV file-type
			//HRESULT hr = m_xAudio2->CreateSourceVoice(&m_sound[indexValue].sourceVoice, (WAVEFORMATEX*)Application::getInstance()->getResourceManager().getAudioData(filename).getFormat());

			//// THIS IS THE OTHER VERSION FOR ADPC
			//		// ... for ADPC-WAV compressed file-type
			//		//hr = xAudio->CreateSourceVoice(&pSourceVoice, (WAVEFORMATEX*)& adpcwf);

			//if (hr != S_OK) {
			//	Logger::Error("Failed to create the actual 'SourceVoice' for a sound file!");
			//	return -1;
			//}

			//hr = m_sound[indexValue].sourceVoice->SubmitSourceBuffer(Application::getInstance()->getResourceManager().getAudioData(filename).getSoundBuffer());

			//if (hr != S_OK) {
			//	Logger::Error("Failed to submit the 'sourceBuffer' to the 'sourceVoice' for a sound file!");
			//	return -1;
			//}

			////m_sound[indexValue].sourceVoice->SetVolume(VOL_THIRD);

			//m_sound[indexValue].emitter.OrientFront = listener.OrientFront;
			//m_sound[indexValue].emitter.OrientTop = listener.OrientTop;
			//m_sound[indexValue].emitter.Position = {listener.Position.x, listener.Position.y, listener.Position.z};
			//m_sound[indexValue].emitter.Velocity = listener.Velocity;

			//m_tempDistance += 0.1f;

			//X3DAudioCalculate(m_X3DInstance, &listener, &m_sound[indexValue].emitter,
			//	X3DAUDIO_CALCULATE_MATRIX | X3DAUDIO_CALCULATE_DOPPLER | X3DAUDIO_CALCULATE_LPF_DIRECT /*| X3DAUDIO_CALCULATE_REVERB*/,
			//	&m_DSPSettings);

			////IXAudio2Voice* pDestb4 = nullptr;
			////float* pMatrixb4 = new float[64];
			////m_sound[indexValue].sourceVoice->GetOutputMatrix(pDest, 1, 2, pMatrix);
			////delete[]pMatrixb4;
			// 
			////XAUDIO2_FILTER_PARAMETERS FilterParameters = {
			////	LowPassFilter,
			////	/*2 * sin(pi * (desired filter cutoff frequency) / sampleRate) */
			////	2.0f * sinf(X3DAUDIO_PI * 20000 / 48000), //6.0f * m_DSPSettings.LPFDirectCoefficient
			////	1.0f
			////};

			//hr = m_sound[indexValue].sourceVoice->SetOutputMatrix(m_masterVoice, 2, m_destinationChannelCount, m_DSPSettings.pMatrixCoefficients);
			//if (hr != S_OK) {
			//	Logger::Error("UN-NAMED ERROR3!");
			//	return -1;
			//}

			//if (m_xAudio2->CommitChanges(XAUDIO2_COMMIT_ALL) == S_OK) {
			//	int asdf = 3;
			//}
			//else {
			//	int asdf = 3;
			//}

			//float* pMatrix = new float[2];
			//m_sound[indexValue].sourceVoice->GetOutputMatrix(NULL, 2, 2, pMatrix);
			//delete[]pMatrix;
			////m_sound[indexValue].sourceVoice->SetFrequencyRatio(m_DSPSettings.DopplerFactor);

			//// R E V E R B   A P P L I C A T I O N
			////m_sound[returnValue].sourceVoice->SetOutputMatrix(m_masterVoice, 1, m_destinationChannelCount, &DSPSettings.ReverbLevel);
			//// EXAMPLE-CODE VERSION: pSFXSourceVoice->SetOutputMatrix(pSubmixVoice, 1, 1, &DSPSettings.ReverbLevel);



			////hr = m_xAudio2->CommitChanges(XAUDIO2_COMMIT_ALL);
			////if (hr != S_OK) {
			////	Logger::Error("UN-NAMED ERROR5!");
			////	return -1;
			////}
			//XAUDIO2_FILTER_PARAMETERS work{
			//	LowPassFilter,
			//	1.0f,
			//	1.0f,
			//};
			//// IFAN, USE THE SOURCE (it's broken)
			//hr = m_sound[indexValue].sourceVoice->SetFilterParameters(&work/*, XAUDIO2_COMMIT_NOW*/);
			//if (hr != S_OK) {
			//	Logger::Error("UN-NAMED ERROR4!");
			//	return -1;
			//}

			//hr = m_sound[indexValue].sourceVoice->Start(0, XAUDIO2_COMMIT_ALL);
			//if (m_xAudio2->CommitChanges(XAUDIO2_COMMIT_ALL) == S_OK) {
			//	int asdf = 3;
			//}

			//if (hr != S_OK) {
			//	Logger::Error("Failed submit processed audio data to data buffer for a audio file");
			//	return -1;
			//}

			//std::cout << "LEFT_EAR_VOL: " << m_DSPSettings.pMatrixCoefficients[0] << "\n";
			//std::cout << "RIGHT_EAR_VOL: " << m_DSPSettings.pMatrixCoefficients[1] << "\n";
			//std::cout << "DISTANCE: " << m_DSPSettings.EmitterToListenerDistance << "\n";
			//float tempFloat;
			//m_sound[indexValue].sourceVoice->GetVolume(&tempFloat);
			//std::cout << "CURRENT VOLUME: " << tempFloat << "\n";

			//return indexValue;
		}
	}

	else {
		Logger::Error("That audio file has NOT been loaded yet!");
		return (-1);
	}
}


// TODO: move things from above back into this function.
int AudioEngine::playSound(const std::string& filename) {
	if (m_masterVoice == nullptr) {
		Logger::Error("'IXAudio2MasterVoice' has not been correctly initialized; audio is unplayable!");
		return -1;
	}
	if (Application::getInstance()->getResourceManager().hasAudioData(filename)) {
		int indexValue = m_currSoundIndex; // Store early
		m_currSoundIndex++;
		m_currSoundIndex %= SOUND_COUNT;

		if (m_sound[indexValue].sourceVoice != nullptr) {
			m_sound[indexValue].sourceVoice->Stop();
			m_sound[indexValue].sourceVoice->DestroyVoice();
		}

		// TODO: check the flags
		//auto hr = m_sound[indexValue].sourceVoice->Start(0, XAUDIO2_COMMIT_ALL);
		auto hr = m_sound[indexValue].sourceVoice->Start();


		{
			//m_sound[indexValue].sourceVoice->SetVolume(VOL_THIRD);

			//m_sound[indexValue].emitter.OrientFront = listener.OrientFront;
			//m_sound[indexValue].emitter.OrientTop = listener.OrientTop;
			//m_sound[indexValue].emitter.Position = { listener.Position.x, listener.Position.y, listener.Position.z };
			//m_sound[indexValue].emitter.Velocity = listener.Velocity;

			//m_tempDistance += 0.1f;

			//X3DAudioCalculate(m_X3DInstance, &listener, &m_sound[indexValue].emitter,
			//	X3DAUDIO_CALCULATE_MATRIX | X3DAUDIO_CALCULATE_DOPPLER | X3DAUDIO_CALCULATE_LPF_DIRECT /*| X3DAUDIO_CALCULATE_REVERB*/,
			//	&m_DSPSettings);

			//IXAudio2Voice* pDestb4 = nullptr;
			//float* pMatrixb4 = new float[64];
			//m_sound[indexValue].sourceVoice->GetOutputMatrix(pDest, 1, 2, pMatrix);
			//delete[]pMatrixb4;

			//XAUDIO2_FILTER_PARAMETERS FilterParameters = {
			//	LowPassFilter,
			//	/*2 * sin(pi * (desired filter cutoff frequency) / sampleRate) */
			//	2.0f * sinf(X3DAUDIO_PI * 20000 / 48000), //6.0f * m_DSPSettings.LPFDirectCoefficient
			//	1.0f
			//};

			//hr = m_sound[indexValue].sourceVoice->SetOutputMatrix(m_masterVoice, 2, m_destinationChannelCount, m_DSPSettings.pMatrixCoefficients);
			//if (hr != S_OK) {
			//	Logger::Error("UN-NAMED ERROR3!");
			//	return -1;
			//}

			//if (m_xAudio2->CommitChanges(XAUDIO2_COMMIT_ALL) == S_OK) {
			//	int asdf = 3;
			//} else {
			//	int asdf = 3;
			//}

			//float* pMatrix = new float[2];
			//m_sound[indexValue].sourceVoice->GetOutputMatrix(NULL, 2, 2, pMatrix);
			//delete[]pMatrix;
			//m_sound[indexValue].sourceVoice->SetFrequencyRatio(m_DSPSettings.DopplerFactor);

			// R E V E R B   A P P L I C A T I O N
			//m_sound[returnValue].sourceVoice->SetOutputMatrix(m_masterVoice, 1, m_destinationChannelCount, &DSPSettings.ReverbLevel);
			// EXAMPLE-CODE VERSION: pSFXSourceVoice->SetOutputMatrix(pSubmixVoice, 1, 1, &DSPSettings.ReverbLevel);



			//hr = m_xAudio2->CommitChanges(XAUDIO2_COMMIT_ALL);
			//if (hr != S_OK) {
			//	Logger::Error("UN-NAMED ERROR5!");
			//	return -1;
			//}
			/*XAUDIO2_FILTER_PARAMETERS work{
				LowPassFilter,
				1.0f,
				1.0f,
			};*/
			// IFAN, USE THE SOURCE (it's broken)
			//hr = m_sound[indexValue].sourceVoice->SetFilterParameters(&work/*, XAUDIO2_COMMIT_NOW*/);
			//if (hr != S_OK) {
			//	Logger::Error("UN-NAMED ERROR4!");
			//	return -1;
			//}

			//hr = m_sound[indexValue].sourceVoice->Start(0, XAUDIO2_COMMIT_ALL);
			//if (m_xAudio2->CommitChanges(XAUDIO2_COMMIT_ALL) == S_OK) {
			//	int asdf = 3;
			//}

			//if (hr != S_OK) {
			//	Logger::Error("Failed submit processed audio data to data buffer for a audio file");
			//	return -1;
			//}

			//std::cout << "LEFT_EAR_VOL: " << m_DSPSettings.pMatrixCoefficients[0] << "\n";
			//std::cout << "RIGHT_EAR_VOL: " << m_DSPSettings.pMatrixCoefficients[1] << "\n";
			//std::cout << "DISTANCE: " << m_DSPSettings.EmitterToListenerDistance << "\n";
			//float tempFloat;
			//m_sound[indexValue].sourceVoice->GetVolume(&tempFloat);
			//std::cout << "CURRENT VOLUME: " << tempFloat << "\n";
		}
		return indexValue;


		{

		//int indexValue = m_currSoundIndex; // Store early
		//m_currSoundIndex++;
		//m_currSoundIndex %= SOUND_COUNT;

		//if (m_sound[indexValue].sourceVoice != nullptr) {
		//	m_sound[indexValue].sourceVoice->Stop();
		//	m_sound[indexValue].sourceVoice->DestroyVoice();
		//}

		//// creating a 'sourceVoice' for WAV file-type
		//HRESULT hr = m_xAudio2->CreateSourceVoice(&m_sound[indexValue].sourceVoice, (WAVEFORMATEX*)Application::getInstance()->getResourceManager().getAudioData(filename).getFormat());

		//// THIS IS THE OTHER VERSION FOR ADPC
		//		// ... for ADPC-WAV compressed file-type
		//		//hr = xAudio->CreateSourceVoice(&pSourceVoice, (WAVEFORMATEX*)& adpcwf);

		//if (hr != S_OK) {
		//	Logger::Error("Failed to create the actual 'SourceVoice' for a sound file!");
		//	return -1;
		//}

		//hr = m_sound[indexValue].sourceVoice->SubmitSourceBuffer(Application::getInstance()->getResourceManager().getAudioData(filename).getSoundBuffer());

		//if (hr != S_OK) {
		//	Logger::Error("Failed to submit the 'sourceBuffer' to the 'sourceVoice' for a sound file!");
		//	return -1;
		//}

		////m_sound[indexValue].sourceVoice->SetVolume(VOL_THIRD);

		//m_sound[indexValue].emitter.OrientFront = listener.OrientFront;
		//m_sound[indexValue].emitter.OrientTop = listener.OrientTop;
		//m_sound[indexValue].emitter.Position = {listener.Position.x, listener.Position.y, listener.Position.z};
		//m_sound[indexValue].emitter.Velocity = listener.Velocity;

		//m_tempDistance += 0.1f;

		//X3DAudioCalculate(m_X3DInstance, &listener, &m_sound[indexValue].emitter,
		//	X3DAUDIO_CALCULATE_MATRIX | X3DAUDIO_CALCULATE_DOPPLER | X3DAUDIO_CALCULATE_LPF_DIRECT /*| X3DAUDIO_CALCULATE_REVERB*/,
		//	&m_DSPSettings);

		////IXAudio2Voice* pDestb4 = nullptr;
		////float* pMatrixb4 = new float[64];
		////m_sound[indexValue].sourceVoice->GetOutputMatrix(pDest, 1, 2, pMatrix);
		////delete[]pMatrixb4;
		// 
		////XAUDIO2_FILTER_PARAMETERS FilterParameters = {
		////	LowPassFilter,
		////	/*2 * sin(pi * (desired filter cutoff frequency) / sampleRate) */
		////	2.0f * sinf(X3DAUDIO_PI * 20000 / 48000), //6.0f * m_DSPSettings.LPFDirectCoefficient
		////	1.0f
		////};

		//hr = m_sound[indexValue].sourceVoice->SetOutputMatrix(m_masterVoice, 2, m_destinationChannelCount, m_DSPSettings.pMatrixCoefficients);
		//if (hr != S_OK) {
		//	Logger::Error("UN-NAMED ERROR3!");
		//	return -1;
		//}

		//if (m_xAudio2->CommitChanges(XAUDIO2_COMMIT_ALL) == S_OK) {
		//	int asdf = 3;
		//}
		//else {
		//	int asdf = 3;
		//}

		//float* pMatrix = new float[2];
		//m_sound[indexValue].sourceVoice->GetOutputMatrix(NULL, 2, 2, pMatrix);
		//delete[]pMatrix;
		////m_sound[indexValue].sourceVoice->SetFrequencyRatio(m_DSPSettings.DopplerFactor);

		//// R E V E R B   A P P L I C A T I O N
		////m_sound[returnValue].sourceVoice->SetOutputMatrix(m_masterVoice, 1, m_destinationChannelCount, &DSPSettings.ReverbLevel);
		//// EXAMPLE-CODE VERSION: pSFXSourceVoice->SetOutputMatrix(pSubmixVoice, 1, 1, &DSPSettings.ReverbLevel);



		////hr = m_xAudio2->CommitChanges(XAUDIO2_COMMIT_ALL);
		////if (hr != S_OK) {
		////	Logger::Error("UN-NAMED ERROR5!");
		////	return -1;
		////}
		//XAUDIO2_FILTER_PARAMETERS work{
		//	LowPassFilter,
		//	1.0f,
		//	1.0f,
		//};
		//// IFAN, USE THE SOURCE (it's broken)
		//hr = m_sound[indexValue].sourceVoice->SetFilterParameters(&work/*, XAUDIO2_COMMIT_NOW*/);
		//if (hr != S_OK) {
		//	Logger::Error("UN-NAMED ERROR4!");
		//	return -1;
		//}

		//hr = m_sound[indexValue].sourceVoice->Start(0, XAUDIO2_COMMIT_ALL);
		//if (m_xAudio2->CommitChanges(XAUDIO2_COMMIT_ALL) == S_OK) {
		//	int asdf = 3;
		//}

		//if (hr != S_OK) {
		//	Logger::Error("Failed submit processed audio data to data buffer for a audio file");
		//	return -1;
		//}

		//std::cout << "LEFT_EAR_VOL: " << m_DSPSettings.pMatrixCoefficients[0] << "\n";
		//std::cout << "RIGHT_EAR_VOL: " << m_DSPSettings.pMatrixCoefficients[1] << "\n";
		//std::cout << "DISTANCE: " << m_DSPSettings.EmitterToListenerDistance << "\n";
		//float tempFloat;
		//m_sound[indexValue].sourceVoice->GetVolume(&tempFloat);
		//std::cout << "CURRENT VOLUME: " << tempFloat << "\n";

		//return indexValue;
		}
	}

	else {
		Logger::Error("That audio file has NOT been loaded yet!");
		return (-1);
	}
}

void AudioEngine::streamSound(const std::string& filename, int streamIndex, bool loop) {
	bool expectedValue = false;
	while (!m_streamLocks[streamIndex].compare_exchange_strong(expectedValue, true));

	if (m_masterVoice == nullptr) {
		Logger::Error("'IXAudio2MasterVoice' has not been correctly initialized; audio is unplayable!");
		m_streamLocks[streamIndex].store(false);
	}
	else {
		this->streamSoundInternal(filename, streamIndex, loop);
	}

	return;
}

void AudioEngine::updateSoundWithCurrentPosition(int index, Camera& cam, Transform& transform, float alpha) {
	glm::vec3 soundPos = transform.getInterpolatedTranslation(alpha);

	// If the sound has an offset position from the entity's transform then rotate the offset with the transform's rotation and add it to the position
	if (m_sound[index].positionOffset != glm::vec3(0, 0, 0)) {
		soundPos += glm::rotate(transform.getInterpolatedRotation(alpha), m_sound[index].positionOffset);
	}

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
	m_sound[index].hrtfParams->SetSourcePosition(&hrtfPosition);
}


void AudioEngine::stopSpecificSound(int index) {

	if (this->checkSoundIndex(index)) {
		if (m_sound[index].sourceVoice != nullptr) {
			m_sound[index].sourceVoice->Stop();
		}
	}
}

void AudioEngine::stopSpecificStream(int index) {

	if (this->checkStreamIndex(index)) {
		if (m_stream[index].sourceVoice != nullptr) {
			m_isStreaming[index] = false;
		}
	}
}

void AudioEngine::stopAllStreams() {

	for (int i = 0; i < STREAMED_SOUNDS_COUNT; i++) {
		m_isStreaming[i] = false;
	}
}

void AudioEngine::stopAllSounds() {

	for (int i = 0; i < SOUND_COUNT; i++) {
		if (m_sound[i].sourceVoice != nullptr) {
			m_sound[i].sourceVoice->Stop();
			m_sound[i].sourceVoice->DestroyVoice();
			m_sound[i].sourceVoice = nullptr;
		}
	}

	for (int i = 0; i < STREAMED_SOUNDS_COUNT; i++) {
		m_isStreaming[i] = false;
	}
}

float AudioEngine::getSoundVolume(int index) {

	float returnValue = 0.0f;

	if (this->checkSoundIndex(index)) {
		m_sound[index].sourceVoice->GetVolume(&returnValue);
	}
	else {
		returnValue = -1.0f;
	}

	return returnValue;
}

float AudioEngine::getStreamVolume(int index) {

	float returnValue = 0.0f;

	if (this->checkStreamIndex(index)) {
		m_stream[index].sourceVoice->GetVolume(&returnValue);
	}
	else {
		returnValue = -1.0f;
	}

	return returnValue;
}

int AudioEngine::getSoundIndex() {
	return m_currSoundIndex;
}

int AudioEngine::getAvailableStreamIndex() {
	int returnValue = -1;

	for (int i = 0; i < STREAMED_SOUNDS_COUNT; i++) {
		if (m_isFinished[i]) {
			m_isStreaming[i] = true;
			returnValue = i;
			break;
		}
	}

	return returnValue;
}

void AudioEngine::setSoundVolume(int index, float value) {

	if (this->checkSoundIndex(index)) {
		m_sound[index].sourceVoice->SetVolume(value);
	}
}

void AudioEngine::setStreamVolume(int index, float value) {

	if (this->checkStreamIndex(index)) {
		m_stream[index].sourceVoice->SetVolume(value);
	}
}

void AudioEngine::initialize() {

	// Init soundObjects
	for (int i = 0; i < SOUND_COUNT; i++) {
		m_sound[i].sourceVoice = nullptr;

		// Initialize EMITTER-structure
		//m_sound[i].emitter.OrientFront = { 0,0,0 };
		//m_sound[i].emitter.OrientTop = { 0,0,0 };
		//m_sound[i].emitter.Position = { 0,0,0 };
		//m_sound[i].emitter.Velocity = { 0,0,0 };
		//m_sound[i].emitter.InnerRadius = 2.0f;
		//m_sound[i].emitter.InnerRadiusAngle = 1.0f;
		//m_sound[i].emitter.ChannelCount = 1;
		//m_sound[i].emitter.ChannelRadius = 1.0f;
		//m_sound[i].emitter.CurveDistanceScaler = DISTANCE_SCALER;
		//m_sound[i].emitter.DopplerScaler = 1.0f;
	}
	// Init streamObjects
	for (int i = 0; i < STREAMED_SOUNDS_COUNT; i++) {
		m_stream[i].sourceVoice = nullptr;
		m_isStreaming[i] = false;
		m_isFinished[i] = true;
		m_overlapped[i] = { 0 };
		m_streamLocks[i].store(false);

		// Initialize EMITTER-structure
		//m_stream[i].emitter.OrientFront = { 0,0,0 };
		//m_stream[i].emitter.OrientTop = { 0,0,0 };
		//m_stream[i].emitter.Position = { 0,0,0 };
		//m_stream[i].emitter.Velocity = { 0,0,0 };
		//m_stream[i].emitter.InnerRadius = 1;
		//m_stream[i].emitter.InnerRadiusAngle = 1;
		//m_stream[i].emitter.ChannelCount = 1;
		//m_stream[i].emitter.ChannelRadius = 1.0f;
		//m_stream[i].emitter.CurveDistanceScaler = DISTANCE_SCALER;
		//m_stream[i].emitter.DopplerScaler = 1.0f;
	}


	if (FAILED(this->initXAudio2())) {
		Logger::Error("Failed to init XAudio2!");
	}



	//this->initXAudio3D();
}

HRESULT AudioEngine::initXAudio2() {
	HRESULT hr = XAudio2Create(&m_xAudio2, XAUDIO2_1024_QUANTUM);

	// HRTF APO expects mono audio data at 48kHz and produces stereo output at 48kHz
	// so we create a stereo mastering voice with specific rendering sample rate of 48kHz.
	// Mastering voice will be automatically destroyed when XAudio2 instance is destroyed.
	if (SUCCEEDED(hr)) {
		hr = m_xAudio2->CreateMasteringVoice(&m_masterVoice, 2, 48000);
	}


	return hr;


	//if (FAILED(hr)) {
	//	Logger::Error("Creating the 'IXAudio2' object failed!");
	//}
	//if (FAILED(hr)) {
	//	Logger::Error("Creating the 'IXAudio2MasterVoice' failed!");
	//}
	//hr = m_xAudio2->CreateMasteringVoice(&m_masterVoice/*, 2, 48000, 0, nullptr, nullptr*/);
	//if (hr != S_OK) {
	//	Logger::Error("Creating the 'IXAudio2MasterVoice' failed!");
	//}
}

//void AudioEngine::initXAudio3D() {
//
//	HRESULT hr;
//	DWORD channelMaskHolder;
//	hr = m_masterVoice->GetChannelMask(&channelMaskHolder);
//	if (hr != S_OK) {
//		Logger::Error("UN-NAMED ERROR1!");
//	}
//
//	XAUDIO2_VOICE_DETAILS tempVoiceDetails;
//	m_masterVoice->GetVoiceDetails(&tempVoiceDetails);
//	m_destinationChannelCount = tempVoiceDetails.InputChannels;
//
//	FLOAT32* matrixCoeff = SAIL_NEW FLOAT32[m_destinationChannelCount];
//	FLOAT32* delayTimes = SAIL_NEW FLOAT32[m_destinationChannelCount];
//	*matrixCoeff = FLOAT32{ 0 };
//	*delayTimes = FLOAT32{ 0 };
//
//	m_DSPSettings.SrcChannelCount = 2;
//	m_DSPSettings.DstChannelCount = m_destinationChannelCount;
//	m_DSPSettings.pMatrixCoefficients = matrixCoeff;
//	m_DSPSettings.pDelayTimes = delayTimes;
//
//	hr = X3DAudioInitialize(channelMaskHolder, SPEED_OF_SOUND, m_X3DInstance);
//	if (hr != S_OK) {
//		Logger::Error("UN-NAMED ERROR2!");
//	}
//}

void AudioEngine::streamSoundInternal(const std::string& filename, int myIndex, bool loop) {

	if (m_isRunning) {

#pragma region VARIABLES_LIST
		// Set to 'officially streaming'
		m_isStreaming[myIndex] = true;
		m_isFinished[myIndex] = false;
		// Release the 'streamLock' AFTER we've set 'm_isStreaming[myIndex] = true'
		m_streamLocks[myIndex].store(false);

		WCHAR wavebank[MAX_PATH];
		DirectX::WaveBankReader wbr;
		StreamingVoiceContext voiceContext;
		HRESULT hr;
		char formatBuff[64];
		WAVEFORMATEX* wfx = nullptr;
		DirectX::WaveBankReader::Metadata metadata;
		std::unique_ptr<uint8_t[]> buffers[MAX_BUFFER_COUNT];
		float currentVolume = 0.0f;
		int totalChunks = 0;
		int currentChunk = 0;
#pragma endregion

		hr = FindMediaFileCch(wavebank, MAX_PATH, stringToWString(filename).c_str());
		if (hr != S_OK) {
			Logger::Error("Failed to find the specified '.xwb' file!");
			return;
		}

		hr = wbr.Open(wavebank);
		if (hr != S_OK) {
			Logger::Error("Failed to open wavebank file!");
			return;
		}

		if (!wbr.IsStreamingBank()) {
			Logger::Error("Tried to stream a non-streamable '.xwb' file! Contact Oliver if you've gotten this message!");
			return;
		}

		while (m_isStreaming[myIndex]) {
			// For every piece of audio within the '.xwb' file (preferably only be 1)
			for (DWORD i = 0; i < wbr.Count(); i++) {

				// Get info we need to play this wave (need space fo PCM, ADPCM, and xWMA formats)
				wfx = reinterpret_cast<WAVEFORMATEX*>(&formatBuff);
				hr = wbr.GetFormat(i, wfx, 64);
				if (hr != S_OK) {
					Logger::Error("Failed to get wave format for '.xwb' file!");
				}

				hr = wbr.GetMetadata(i, metadata);
				if (hr != S_OK) {
					Logger::Error("Failed to get meta data for '.xwb' file!");
				}

				hr = m_xAudio2->CreateSourceVoice(&m_stream[myIndex].sourceVoice, wfx, 0, 1.0f, &voiceContext);
				if (hr != S_OK) {
					Logger::Error("Failed to create source voice!");
				}

				m_stream[myIndex].sourceVoice->SetVolume(0);

				// Create the 'overlapped' structure as well as buffers to handle async I/O
#if (_WIN32_WINNT >= _WIN32_WINNT_VISTA)
				m_overlapped[myIndex].hEvent = CreateEventEx(nullptr, nullptr, CREATE_EVENT_MANUAL_RESET, EVENT_MODIFY_STATE | SYNCHRONIZE);
#else
				m_overlapped.hEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
#endif

				if ((STREAMING_BUFFER_SIZE % wfx->nBlockAlign) != 0)
				{
					//
					// non-PCM data will fail here. ADPCM requires a more complicated streaming mechanism to deal with submission in audio frames that do
					// not necessarily align to the 2K async boundary.
					//
					m_isStreaming[myIndex] = false;
					break;
				}

				for (size_t j = 0; j < MAX_BUFFER_COUNT; ++j)
				{
					buffers[j].reset(SAIL_NEW uint8_t[STREAMING_BUFFER_SIZE]);
				}

				DWORD currentDiskReadBuffer = 0;
				DWORD currentPosition = 0;

				HANDLE async = wbr.GetAsyncHandle();

				// Reading from the file (when time-since-last-read has passed threshold)
				while ((currentPosition < metadata.lengthBytes) && m_isStreaming[myIndex])
				{
					if (GetAsyncKeyState(VK_ESCAPE))
					{
						m_isStreaming[myIndex] = false;
						while (GetAsyncKeyState(VK_ESCAPE) && m_isStreaming[myIndex]) {
							Sleep(10);
						}
						break;
					}

					DWORD cbValid = std::min(STREAMING_BUFFER_SIZE, static_cast<int>(metadata.lengthBytes - static_cast<UINT32>(currentPosition)));
					m_overlapped[myIndex].Offset = metadata.offsetBytes + currentPosition;

					bool wait = false;
					if (!ReadFile(async, buffers[currentDiskReadBuffer].get(), STREAMING_BUFFER_SIZE, nullptr, &m_overlapped[myIndex]))
					{
						DWORD error = GetLastError();
						if (error != ERROR_IO_PENDING)
						{
							m_isStreaming[myIndex] = false;
							break;
						}
						wait = true;
					}

					currentPosition += cbValid;

					//
					// At this point the read is progressing in the background and we are free to do
					// other processing while we wait for it to finish. For the purposes of this sample,
					// however, we'll just go to sleep until the read is done.
					//
					if (wait) {
						WaitForSingleObject(m_overlapped[myIndex].hEvent, INFINITE);
					}

					DWORD cb;
#if (_WIN32_WINNT >= 0x0602 /*_WIN32_WINNT_WIN8*/)
					BOOL result = GetOverlappedResultEx(async, &m_overlapped[myIndex], &cb, 0, FALSE);
#else
					BOOL result = GetOverlappedResult(async, &ovlCurrentRequest, &cb, FALSE);
#endif

					if (!result)
					{
						m_isStreaming[myIndex] = false;
						break;
					}

					//
					// Now that the event has been signaled, we know we have audio available. The next
					// question is whether our XAudio2 source voice has played enough data for us to give
					// it another buffer full of audio. We'd like to keep no more than MAX_BUFFER_COUNT - 1
					// buffers on the queue, so that one buffer is always free for disk I/O.
					//
					XAUDIO2_VOICE_STATE state;
					for (;;)
					{
						m_stream[myIndex].sourceVoice->GetState(&state);
						if (state.BuffersQueued < MAX_BUFFER_COUNT - 1) {
							break;
						}

						m_stream[myIndex].sourceVoice->Start();
						if (currentVolume < VOL_THIRD) {
							currentVolume += 0.1f;
							m_stream[myIndex].sourceVoice->SetVolume(currentVolume);
						}

						currentChunk++;
						WaitForSingleObject(voiceContext.hBufferEndEvent, INFINITE);
					}

					//
					// At this point we have a buffer full of audio and enough room to submit it, so
					// let's submit it and get another read request going.
					//
					XAUDIO2_BUFFER buf = { 0 };
					buf.AudioBytes = cbValid;
					buf.pAudioData = buffers[currentDiskReadBuffer].get();
					if (currentPosition >= metadata.lengthBytes) {
						buf.Flags = XAUDIO2_END_OF_STREAM;
					}

					m_stream[myIndex].sourceVoice->SubmitSourceBuffer(&buf);

					currentDiskReadBuffer++;
					currentDiskReadBuffer %= MAX_BUFFER_COUNT;
				}
			}

			if (!loop) {
				m_isStreaming[myIndex] = false;
			}

			//if (!m_isStreaming[myIndex])
			//{
			//	m_sourceVoiceStream[myIndex]->SetVolume(0);

			//	XAUDIO2_VOICE_STATE state;
			//	for (;;)
			//	{
			//		m_sourceVoiceStream[myIndex]->GetState(&state);
			//		if (!state.BuffersQueued)
			//			break;

			//		WaitForSingleObject(voiceContext.hBufferEndEvent, INFINITE);
			//	}
			//}

			currentChunk = 0;
			currentVolume = 0;
			m_stream[myIndex].sourceVoice->Stop();
		}
		//
		// Clean up
		//
		if (m_stream[myIndex].sourceVoice != nullptr) {
			m_stream[myIndex].sourceVoice->Stop();
			m_stream[myIndex].sourceVoice->DestroyVoice();
			m_stream[myIndex].sourceVoice = nullptr;
			CloseHandle(m_overlapped[myIndex].hEvent);
		}

		m_isFinished[myIndex] = true;
		m_streamLocks[myIndex].store(false);
	}

	return;
}

//--------------------------------------------------------------------------------------
// Helper function to try to find the location of a media file
//--------------------------------------------------------------------------------------
_Use_decl_annotations_
HRESULT AudioEngine::FindMediaFileCch(WCHAR* strDestPath, int cchDest, LPCWSTR strFilename)
{
	bool bFound = false;

	if (!strFilename || strFilename[0] == 0 || !strDestPath || cchDest < 10)
		return E_INVALIDARG;

	// Get the exe name, and exe path
	WCHAR strExePath[MAX_PATH] = { 0 };
	WCHAR strExeName[MAX_PATH] = { 0 };
	WCHAR* strLastSlash = nullptr;
	GetModuleFileName(nullptr, strExePath, MAX_PATH);
	strExePath[MAX_PATH - 1] = 0;
	strLastSlash = wcsrchr(strExePath, TEXT('\\'));
	if (strLastSlash)
	{
		wcscpy_s(strExeName, MAX_PATH, &strLastSlash[1]);

		// Chop the exe name from the exe path
		*strLastSlash = 0;

		// Chop the .exe from the exe name
		strLastSlash = wcsrchr(strExeName, TEXT('.'));
		if (strLastSlash)
			* strLastSlash = 0;
	}

	wcscpy_s(strDestPath, cchDest, strFilename);
	if (GetFileAttributes(strDestPath) != 0xFFFFFFFF)
		return S_OK;

	// Search all parent directories starting at .\ and using strFilename as the leaf name
	WCHAR strLeafName[MAX_PATH] = { 0 };
	wcscpy_s(strLeafName, MAX_PATH, strFilename);

	WCHAR strFullPath[MAX_PATH] = { 0 };
	WCHAR strFullFileName[MAX_PATH] = { 0 };
	WCHAR strSearch[MAX_PATH] = { 0 };
	WCHAR* strFilePart = nullptr;

	GetFullPathName(L".", MAX_PATH, strFullPath, &strFilePart);
	if (!strFilePart)
		return E_FAIL;

	while (strFilePart && *strFilePart != '\0')
	{
		swprintf_s(strFullFileName, MAX_PATH, L"%s\\%s", strFullPath, strLeafName);
		if (GetFileAttributes(strFullFileName) != 0xFFFFFFFF)
		{
			wcscpy_s(strDestPath, cchDest, strFullFileName);
			bFound = true;
			break;
		}

		swprintf_s(strFullFileName, MAX_PATH, L"%s\\%s\\%s", strFullPath, strExeName, strLeafName);
		if (GetFileAttributes(strFullFileName) != 0xFFFFFFFF)
		{
			wcscpy_s(strDestPath, cchDest, strFullFileName);
			bFound = true;
			break;
		}

		swprintf_s(strSearch, MAX_PATH, L"%s\\..", strFullPath);
		GetFullPathName(strSearch, MAX_PATH, strFullPath, &strFilePart);
	}
	if (bFound)
		return S_OK;

	// On failure, return the file as the path but also return an error code
	wcscpy_s(strDestPath, cchDest, strFilename);

	return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
}

bool AudioEngine::checkSoundIndex(int index) {

	if (index < 0 || index > SOUND_COUNT) {
		Logger::Error("Tried to STOP a sound from playing with an INVALID INDEX!");
		return false;
	}

	else {
		return true;
	}
}

bool AudioEngine::checkStreamIndex(int index) {

	if (index < 0 || index > STREAMED_SOUNDS_COUNT) {
		Logger::Error("Tried to STOP a sound from being streamed with an INVALID INDEX!");
		return false;
	}

	else {
		return true;
	}
}
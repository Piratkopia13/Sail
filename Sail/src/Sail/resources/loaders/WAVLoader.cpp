#include "pch.h"
#include "WAVLoader.h"
//#include "../../../API/Audio/GeneralFunctions.h"
#include "API/Audio/GeneralFunctions.h"

namespace Fileloader {

	WAVLoader::WAVLoader(const std::string& filename, IXAudio2* xAudio2, ResourceFormat::AudioData& audioData) {
		std::string fileExt = "";
		fileExt = getFileExtension(filename);

		bool result = false;

		if (fileExt == "wav") {
			result = this->loadWAV(filename, xAudio2, audioData);
		}

		else {
			Logger::Warning("The audio file extension \"" + fileExt + "\" could not be read!");
		}

		if (!result) {
			Logger::Warning("Sound file \"" + filename + "\" could not be read!");
		}
	}

	WAVLoader::~WAVLoader() {

	}

	bool WAVLoader::loadWAV(const std::string& filename, IXAudio2* xAudio2, ResourceFormat::AudioData& audioData) {

		// PROBABLY ISN'T NEEDED ANYMORE!
		//-------------------------------
		//if (audioData.m_soundBuffer.pAudioData != nullptr) {
		//	delete audioData.m_soundBuffer.pAudioData;
		//}

		HRESULT hr = 0;

		std::wstring fileNameWSTR = stringToWString(filename);
		LPCWSTR fileNameLPCWSTR = fileNameWSTR.c_str();

		HANDLE hFile = CreateFile(
			fileNameLPCWSTR,
			GENERIC_READ,
			FILE_SHARE_READ,
			NULL,
			OPEN_EXISTING,
			0,
			nullptr);

		DWORD dwChunkSize = 0;
		DWORD dwChunkPosition = 0;

		DWORD filetype = 0;

		if (hr != S_OK) {
			Logger::Error("Failed to create the internal audio file!");
		}

#pragma region SPECIAL_CASE_ERROR_CHECK
		try {
			if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, 0, NULL, FILE_BEGIN)) {
				throw std::invalid_argument(nullptr);
			}
		}
		catch (const std::invalid_argument& e) {

			UNREFERENCED_PARAMETER(e);
			wchar_t errorMsgBuffer[256];
			wsprintfW(errorMsgBuffer, L"FUNCTION: Audio::loadSound()\n\nMESSAGE: 'SetFilePointer' failed!");
			MessageBox(NULL, errorMsgBuffer, static_cast<LPCWSTR>(L"AUDIO ERROR!"), MB_ICONERROR);
			std::exit(0);
		}
#pragma endregion

		hr = findChunk(hFile, fourccRIFF, dwChunkSize, dwChunkPosition);
		if (hr != S_OK) {
			Logger::Error("Failed to find the audio file's 'RIFF' data chunk!");
		}

		hr = readChunkData(hFile, &filetype, sizeof(DWORD), dwChunkPosition);
		if (hr != S_OK) {
			Logger::Error("Failed to read data chunk!");
		}

#pragma region SPECIAL_CASE_ERROR_CHECK
		try {
			if (filetype != fourccWAVE) {
				throw std::invalid_argument(nullptr);
			}
		}
		catch (const std::invalid_argument& e) {

			UNREFERENCED_PARAMETER(e);
			wchar_t errorMsgBuffer[256];
			wsprintfW(errorMsgBuffer, L"FUNCTION: Audio::loadSound()\n\nMESSAGE: The file type was NOT of the 'WAV' type!");
			MessageBox(NULL, errorMsgBuffer, static_cast<LPCWSTR>(L"AUDIO ERROR!"), MB_ICONERROR);
			std::exit(0);
		}
#pragma endregion

		hr = findChunk(hFile, fourccFMT, dwChunkSize, dwChunkPosition);
		if (hr != S_OK) {
			Logger::Error("Failed to find the desired data chunk!");
		}

		// reading data from WAV files
		hr = readChunkData(hFile, (void*)& audioData.m_formatWAV, dwChunkSize, dwChunkPosition);
		// reading data from ADPC-WAV files (compressed)
		/*hr = ReadChunkData(hFile, &adpcwf, dwChunkSize, dwChunkPosition);*/
		if (hr != S_OK) {
			Logger::Error("Failed to read data chunk!");
		}

		hr = findChunk(hFile, fourccDATA, dwChunkSize, dwChunkPosition);
		if (hr != S_OK) {
			Logger::Error("Failed to find the desired data chunk!");
		}

		BYTE* pDataBuffer = new BYTE[dwChunkSize];

		hr = readChunkData(hFile, pDataBuffer, dwChunkSize, dwChunkPosition);
		if (hr != S_OK) {
			Logger::Error("Failed to read data chunk!");
		}

		audioData.m_soundBuffer.AudioBytes = dwChunkSize;  //size of the audio buffer in bytes
		audioData.m_soundBuffer.pAudioData = pDataBuffer;  //buffer containing audio data
		audioData.m_soundBuffer.Flags = XAUDIO2_END_OF_STREAM; // tell the source voice not to expect any data after this buffer
	}

//	bool WAVLoader::loadADPC(std::string filename, IXAudio2* xAudio2, ResourceFormat::AudioData& audioData) {

		//		ADPCMWAVEFORMAT* formatADPCM = { 0 };
		//
		//		// PROBABLY ISN'T NEEDED ANYMORE!
		//		//-------------------------------
		//		//if (audioData.m_soundBuffer.pAudioData != nullptr) {
		//		//	delete audioData.m_soundBuffer.pAudioData;
		//		//}
		//
		//		HRESULT hr = 0;
		//
		//		std::wstring fileNameWSTR = stringToWString(filename);
		//		LPCWSTR fileNameLPCWSTR = fileNameWSTR.c_str();
		//
		//		HANDLE hFile = CreateFile(
		//			fileNameLPCWSTR,
		//			GENERIC_READ,
		//			FILE_SHARE_READ,
		//			NULL,
		//			OPEN_EXISTING,
		//			0,
		//			nullptr);
		//
		//		DWORD dwChunkSize = 0;
		//		DWORD dwChunkPosition = 0;
		//		DWORD filetype = 0;
		//
		//#pragma region ERROR_CHECKING
		//		try {
		//			if (INVALID_HANDLE_VALUE == hFile) {
		//				throw std::invalid_argument(nullptr);
		//			}
		//		}
		//		catch (const std::invalid_argument& e) {
		//
		//			UNREFERENCED_PARAMETER(e);
		//			wchar_t errorMsgBuffer[256];
		//			wsprintfW(errorMsgBuffer, L"FUNCTION: Audio::loadCompressedSound()\n\nMESSAGE: Failed to create the internal audio file!");
		//			MessageBox(NULL, errorMsgBuffer, static_cast<LPCWSTR>(L"AUDIO ERROR!"), MB_ICONERROR);
		//			std::exit(0);
		//		}
		//
		//		try {
		//			if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, 0, NULL, FILE_BEGIN)) {
		//				throw std::invalid_argument(nullptr);
		//			}
		//		}
		//		catch (const std::invalid_argument& e) {
		//
		//			UNREFERENCED_PARAMETER(e);
		//			wchar_t errorMsgBuffer[256];
		//			wsprintfW(errorMsgBuffer, L"FUNCTION: Audio::loadSound()\n\nMESSAGE: 'SetFilePointer' failed!");
		//			MessageBox(NULL, errorMsgBuffer, static_cast<LPCWSTR>(L"AUDIO ERROR!"), MB_ICONERROR);
		//			std::exit(0);
		//		}
		//#pragma endregion
		//
		//		hr = findChunk(hFile, fourccRIFF, dwChunkSize, dwChunkPosition);
		//
		//#pragma region ERROR_CHECKING
		//		try {
		//			if (hr != S_OK) {
		//				throw std::invalid_argument(nullptr);
		//			}
		//		}
		//		catch (const std::invalid_argument& e) {
		//
		//			UNREFERENCED_PARAMETER(e);
		//			wchar_t errorMsgBuffer[256];
		//			wsprintfW(errorMsgBuffer, L"FUNCTION: Audio::loadCompressedSound()\n\nMESSAGE: Failed to find the audio file's 'RIFF' data chunk!");
		//			MessageBox(NULL, errorMsgBuffer, static_cast<LPCWSTR>(L"AUDIO ERROR!"), MB_ICONERROR);
		//			std::exit(0);
		//		}
		//#pragma endregion
		//
		//		hr = readChunkData(hFile, &filetype, sizeof(DWORD), dwChunkPosition);
		//
		//#pragma region ERROR_CHECKING
		//		try {
		//			if (hr != S_OK) {
		//				throw std::invalid_argument(nullptr);
		//			}
		//		}
		//		catch (const std::invalid_argument& e) {
		//
		//			UNREFERENCED_PARAMETER(e);
		//			wchar_t errorMsgBuffer[256];
		//			wsprintfW(errorMsgBuffer, L"FUNCTION: Audio::loadCompressedSound()\n\nMESSAGE: Failed to read data chunk!");
		//			MessageBox(NULL, errorMsgBuffer, static_cast<LPCWSTR>(L"AUDIO ERROR!"), MB_ICONERROR);
		//			std::exit(0);
		//		}
		//
		//		try {
		//			if (filetype != fourccWAVE) {
		//				throw std::invalid_argument(nullptr);
		//			}
		//		}
		//		catch (const std::invalid_argument& e) {
		//
		//			UNREFERENCED_PARAMETER(e);
		//			wchar_t errorMsgBuffer[256];
		//			wsprintfW(errorMsgBuffer, L"FUNCTION: Audio::loadSound()\n\nMESSAGE: The file type was NOT of the 'WAV' type!");
		//			MessageBox(NULL, errorMsgBuffer, static_cast<LPCWSTR>(L"AUDIO ERROR!"), MB_ICONERROR);
		//			std::exit(0);
		//		}
		//#pragma endregion
		//
		//		hr = findChunk(hFile, fourccFMT, dwChunkSize, dwChunkPosition);
		//
		//#pragma region ERROR_CHECKING
		//		try {
		//			if (hr != S_OK) {
		//				throw std::invalid_argument(nullptr);
		//			}
		//		}
		//		catch (const std::invalid_argument& e) {
		//
		//			UNREFERENCED_PARAMETER(e);
		//			wchar_t errorMsgBuffer[256];
		//			wsprintfW(errorMsgBuffer, L"FUNCTION: Audio::loadCompressedSound()\n\nMESSAGE: Failed to find the desired data chunk!");
		//			MessageBox(NULL, errorMsgBuffer, static_cast<LPCWSTR>(L"AUDIO ERROR!"), MB_ICONERROR);
		//			std::exit(0);
		//		}
		//#pragma endregion
		//
		//		// reading data from WAV files
		//		hr = readChunkData(hFile, formatADPCM, dwChunkSize, dwChunkPosition);
		//		// reading data from ADPC-WAV files (compressed)
		//		/*hr = ReadChunkData(hFile, &adpcwf, dwChunkSize, dwChunkPosition);*/
		//
		//#pragma region ERROR_CHECKING
		//		try {
		//			if (hr != S_OK) {
		//				throw std::invalid_argument(nullptr);
		//			}
		//		}
		//		catch (const std::invalid_argument& e) {
		//
		//			UNREFERENCED_PARAMETER(e);
		//			wchar_t errorMsgBuffer[256];
		//			wsprintfW(errorMsgBuffer, L"FUNCTION: Audio::loadCompressedSound()\n\nMESSAGE: Failed to read data chunk!");
		//			MessageBox(NULL, errorMsgBuffer, static_cast<LPCWSTR>(L"AUDIO ERROR!"), MB_ICONERROR);
		//			std::exit(0);
		//		}
		//#pragma endregion
		//
		//		hr = findChunk(hFile, fourccDATA, dwChunkSize, dwChunkPosition);
		//
		//#pragma region ERROR_CHECKING
		//		try {
		//			if (hr != S_OK) {
		//				throw std::invalid_argument(nullptr);
		//			}
		//		}
		//		catch (const std::invalid_argument& e) {
		//
		//			UNREFERENCED_PARAMETER(e);
		//			wchar_t errorMsgBuffer[256];
		//			wsprintfW(errorMsgBuffer, L"FUNCTION: Audio::loadCompressedSound()\n\nMESSAGE: Failed to find the desired data chunk!");
		//			MessageBox(NULL, errorMsgBuffer, static_cast<LPCWSTR>(L"AUDIO ERROR!"), MB_ICONERROR);
		//			std::exit(0);
		//		}
		//#pragma endregion
		//
		//		BYTE* pDataBuffer = new BYTE[dwChunkSize];
		//
		//		hr = readChunkData(hFile, pDataBuffer, dwChunkSize, dwChunkPosition);
		//
		//#pragma region ERROR_CHECKING
		//		try {
		//			if (hr != S_OK) {
		//				throw std::invalid_argument(nullptr);
		//			}
		//		}
		//		catch (const std::invalid_argument& e) {
		//
		//			UNREFERENCED_PARAMETER(e);
		//			wchar_t errorMsgBuffer[256];
		//			wsprintfW(errorMsgBuffer, L"FUNCTION: Audio::loadCompressedSound()\n\nMESSAGE: Failed to read data chunk!");
		//			MessageBox(NULL, errorMsgBuffer, static_cast<LPCWSTR>(L"AUDIO ERROR!"), MB_ICONERROR);
		//			std::exit(0);
		//		}
		//#pragma endregion ERROR_CHECKING
		//
		//		audioData.m_soundBuffer.AudioBytes = dwChunkSize;  //buffer containing audio data
		//		audioData.m_soundBuffer.pAudioData = pDataBuffer;  //size of the audio buffer in bytes
		//		audioData.m_soundBuffer.Flags = XAUDIO2_END_OF_STREAM; // tell the source voice not to expect any data after this buffer
		//
		//		// creating a 'sourceVoice' for WAV file-type
		//		//hr = xAudio2->CreateSourceVoice(&audioData.m_sourceVoice, (WAVEFORMATEX*)& m_formatADPCM);
		//
		//#pragma region ERROR_CHECKING
		//		try {
		//			if (hr != S_OK) {
		//				throw std::invalid_argument(nullptr);
		//			}
		//		}
		//		catch (const std::invalid_argument& e) {
		//
		//			UNREFERENCED_PARAMETER(e);
		//			wchar_t errorMsgBuffer[256];
		//			wsprintfW(errorMsgBuffer, L"FUNCTION: Audio::loadCompressedSound()\n\nMESSAGE: Failed to create the actual 'SourceVoice' for the sound!");
		//			MessageBox(NULL, errorMsgBuffer, static_cast<LPCWSTR>(L"AUDIO ERROR!"), MB_ICONERROR);
		//			std::exit(0);
		//		}
		//#pragma endregion
		//
		//		//hr = audioData.m_sourceVoice->SubmitSourceBuffer(&audioData.m_soundBuffer);
		//
		//#pragma region ERROR_CHECKING
		//		try {
		//			if (hr != S_OK) {
		//				throw std::invalid_argument(nullptr);
		//			}
		//		}
		//		catch (const std::invalid_argument& e) {
		//
		//			UNREFERENCED_PARAMETER(e);
		//			wchar_t errorMsgBuffer[256];
		//			wsprintfW(errorMsgBuffer, L"FUNCTION: Audio::loadCompressedSound()\n\nMESSAGE: Failed submit processed audio data to data buffer!");
		//			MessageBox(NULL, errorMsgBuffer, static_cast<LPCWSTR>(L"AUDIO ERROR!"), MB_ICONERROR);
		//			std::exit(0);
		//		}
		//#pragma endregion
		//	}
//	}
}
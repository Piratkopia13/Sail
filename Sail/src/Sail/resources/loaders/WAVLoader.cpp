#include "pch.h"
#include "WAVLoader.h"

namespace Fileloader {

	WAVLoader::WAVLoader(std::string filename, IXAudio2* xAudio2, ResourceFormat::AudioData& audioData) {
		std::string fileExt = "";
		fileExt = this->getFileExtension(filename);

		bool result = false;

		if (fileExt == "wav") {
			result = this->loadWAV(filename, xAudio2, audioData);
		}

		else if (fileExt == "adpc") {
			result = this->loadADPC(filename, xAudio2, audioData);
		}

		if (!result) {
			Logger::Warning("Texture file \"" + filename + "\" could not be read!");
		}
	}

	WAVLoader::~WAVLoader() {

	}

	bool WAVLoader::loadWAV(std::string filename, IXAudio2* xAudio2, ResourceFormat::AudioData& audioData) {

		WAVEFORMATEXTENSIBLE m_formatWAV = { 0 };

		// PROBABLY ISN'T NEEDED ANYMORE!
		//-------------------------------
		//if (audioData.m_soundBuffer.pAudioData != nullptr) {
		//	delete audioData.m_soundBuffer.pAudioData;
		//}

		HRESULT hr = 0;

		std::wstring fileNameWSTR = s2ws(filename);
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

#pragma region ERROR_CHECKING
		try {
			if (INVALID_HANDLE_VALUE == hFile) {
				throw std::invalid_argument(nullptr);
			}
		}
		catch (const std::invalid_argument& e) {

			UNREFERENCED_PARAMETER(e);
			wchar_t errorMsgBuffer[256];
			wsprintfW(errorMsgBuffer, L"FUNCTION: Audio::loadSound()\n\nMESSAGE: Failed to create the internal audio file!");
			MessageBox(NULL, errorMsgBuffer, static_cast<LPCWSTR>(L"AUDIO ERROR!"), MB_ICONERROR);
			std::exit(0);
		}

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

#pragma region ERROR_CHECKING
		try {
			if (hr != S_OK) {
				throw std::invalid_argument(nullptr);
			}
		}
		catch (const std::invalid_argument& e) {

			UNREFERENCED_PARAMETER(e);
			wchar_t errorMsgBuffer[256];
			wsprintfW(errorMsgBuffer, L"FUNCTION: Audio::loadSound()\n\nMESSAGE: Failed to find the audio file's 'RIFF' data chunk!");
			MessageBox(NULL, errorMsgBuffer, static_cast<LPCWSTR>(L"AUDIO ERROR!"), MB_ICONERROR);
			std::exit(0);
		}
#pragma endregion

		hr = readChunkData(hFile, &filetype, sizeof(DWORD), dwChunkPosition);

#pragma region ERROR_CHECKING
		try {
			if (hr != S_OK) {
				throw std::invalid_argument(nullptr);
			}
		}
		catch (const std::invalid_argument& e) {

			UNREFERENCED_PARAMETER(e);
			wchar_t errorMsgBuffer[256];
			wsprintfW(errorMsgBuffer, L"FUNCTION: Audio::loadSound()\n\nMESSAGE: Failed to read data chunk!");
			MessageBox(NULL, errorMsgBuffer, static_cast<LPCWSTR>(L"AUDIO ERROR!"), MB_ICONERROR);
			std::exit(0);
		}

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

#pragma region ERROR_CHECKING
		try {
			if (hr != S_OK) {
				throw std::invalid_argument(nullptr);
			}
		}
		catch (const std::invalid_argument& e) {

			UNREFERENCED_PARAMETER(e);
			wchar_t errorMsgBuffer[256];
			wsprintfW(errorMsgBuffer, L"FUNCTION: Audio::loadSound()\n\nMESSAGE: Failed to find the desired data chunk!");
			MessageBox(NULL, errorMsgBuffer, static_cast<LPCWSTR>(L"AUDIO ERROR!"), MB_ICONERROR);
			std::exit(0);
		}
#pragma endregion

		// reading data from WAV files
		hr = readChunkData(hFile, &m_formatWAV, dwChunkSize, dwChunkPosition);
		// reading data from ADPC-WAV files (compressed)
		/*hr = ReadChunkData(hFile, &adpcwf, dwChunkSize, dwChunkPosition);*/

#pragma region ERROR_CHECKING
		try {
			if (hr != S_OK) {
				throw std::invalid_argument(nullptr);
			}
		}
		catch (const std::invalid_argument& e) {

			UNREFERENCED_PARAMETER(e);
			wchar_t errorMsgBuffer[256];
			wsprintfW(errorMsgBuffer, L"FUNCTION: Audio::loadSound()\n\nMESSAGE: Failed to read data chunk!");
			MessageBox(NULL, errorMsgBuffer, static_cast<LPCWSTR>(L"AUDIO ERROR!"), MB_ICONERROR);
			std::exit(0);
		}
#pragma endregion

		hr = findChunk(hFile, fourccDATA, dwChunkSize, dwChunkPosition);

#pragma region ERROR_CHECKING
		try {
			if (hr != S_OK) {
				throw std::invalid_argument(nullptr);
			}
		}
		catch (const std::invalid_argument& e) {

			UNREFERENCED_PARAMETER(e);
			wchar_t errorMsgBuffer[256];
			wsprintfW(errorMsgBuffer, L"FUNCTION: Audio::loadSound()\n\nMESSAGE: Failed to find the desired data chunk!");
			MessageBox(NULL, errorMsgBuffer, static_cast<LPCWSTR>(L"AUDIO ERROR!"), MB_ICONERROR);
			std::exit(0);
		}
#pragma endregion

		BYTE* pDataBuffer = new BYTE[dwChunkSize];

		hr = readChunkData(hFile, pDataBuffer, dwChunkSize, dwChunkPosition);

#pragma region ERROR_CHECKING
		try {
			if (hr != S_OK) {
				throw std::invalid_argument(nullptr);
			}
		}
		catch (const std::invalid_argument& e) {

			UNREFERENCED_PARAMETER(e);
			wchar_t errorMsgBuffer[256];
			wsprintfW(errorMsgBuffer, L"FUNCTION: Audio::loadSound()\n\nMESSAGE: Failed to read data chunk!");
			MessageBox(NULL, errorMsgBuffer, static_cast<LPCWSTR>(L"AUDIO ERROR!"), MB_ICONERROR);
			std::exit(0);
		}
#pragma endregion ERROR_CHECKING

		audioData.m_soundBuffer.AudioBytes = dwChunkSize;  //buffer containing audio data
		audioData.m_soundBuffer.pAudioData = pDataBuffer;  //size of the audio buffer in bytes
		audioData.m_soundBuffer.Flags = XAUDIO2_END_OF_STREAM; // tell the source voice not to expect any data after this buffer

		// creating a 'sourceVoice' for WAV file-type
		hr = xAudio2->CreateSourceVoice(&audioData.m_sourceVoice, (WAVEFORMATEX*)& m_formatWAV);
		// creating a 'sourceVoice' for ADPC-WAV compressed file-type
		//hr = xAudio->CreateSourceVoice(&pSourceVoice, (WAVEFORMATEX*)& adpcwf);

#pragma region ERROR_CHECKING
		try {
			if (hr != S_OK) {
				throw std::invalid_argument(nullptr);
			}
		}
		catch (const std::invalid_argument& e) {

			UNREFERENCED_PARAMETER(e);
			wchar_t errorMsgBuffer[256];
			wsprintfW(errorMsgBuffer, L"FUNCTION: Audio::loadSound()\n\nMESSAGE: Failed to create the actual 'SourceVoice' for the sound!");
			MessageBox(NULL, errorMsgBuffer, static_cast<LPCWSTR>(L"AUDIO ERROR!"), MB_ICONERROR);
			std::exit(0);
		}
#pragma endregion

		hr = audioData.m_sourceVoice->SubmitSourceBuffer(&audioData.m_soundBuffer);

#pragma region ERROR_CHECKING
		try {
			if (hr != S_OK) {
				throw std::invalid_argument(nullptr);
			}
		}
		catch (const std::invalid_argument& e) {

			UNREFERENCED_PARAMETER(e);
			wchar_t errorMsgBuffer[256];
			wsprintfW(errorMsgBuffer, L"FUNCTION: Audio::loadSound()\n\nMESSAGE: Failed submit processed audio data to data buffer!");
			MessageBox(NULL, errorMsgBuffer, static_cast<LPCWSTR>(L"AUDIO ERROR!"), MB_ICONERROR);
			std::exit(0);
		}
#pragma endregion

		//m_currIndex++;
	}

	bool WAVLoader::loadADPC(std::string filename, IXAudio2* xAudio2, ResourceFormat::AudioData& audioData) {
		
		ADPCMWAVEFORMAT m_formatADPCM = { 0 };
		
		// PROBABLY ISN'T NEEDED ANYMORE!
		//-------------------------------
		//if (audioData.m_soundBuffer.pAudioData != nullptr) {
		//	delete audioData.m_soundBuffer.pAudioData;
		//}

		HRESULT hr = 0;

		std::wstring fileNameWSTR = s2ws(filename);
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

#pragma region ERROR_CHECKING
		try {
			if (INVALID_HANDLE_VALUE == hFile) {
				throw std::invalid_argument(nullptr);
			}
		}
		catch (const std::invalid_argument& e) {

			UNREFERENCED_PARAMETER(e);
			wchar_t errorMsgBuffer[256];
			wsprintfW(errorMsgBuffer, L"FUNCTION: Audio::loadCompressedSound()\n\nMESSAGE: Failed to create the internal audio file!");
			MessageBox(NULL, errorMsgBuffer, static_cast<LPCWSTR>(L"AUDIO ERROR!"), MB_ICONERROR);
			std::exit(0);
		}

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

#pragma region ERROR_CHECKING
		try {
			if (hr != S_OK) {
				throw std::invalid_argument(nullptr);
			}
		}
		catch (const std::invalid_argument& e) {

			UNREFERENCED_PARAMETER(e);
			wchar_t errorMsgBuffer[256];
			wsprintfW(errorMsgBuffer, L"FUNCTION: Audio::loadCompressedSound()\n\nMESSAGE: Failed to find the audio file's 'RIFF' data chunk!");
			MessageBox(NULL, errorMsgBuffer, static_cast<LPCWSTR>(L"AUDIO ERROR!"), MB_ICONERROR);
			std::exit(0);
		}
#pragma endregion

		hr = readChunkData(hFile, &filetype, sizeof(DWORD), dwChunkPosition);

#pragma region ERROR_CHECKING
		try {
			if (hr != S_OK) {
				throw std::invalid_argument(nullptr);
			}
		}
		catch (const std::invalid_argument& e) {

			UNREFERENCED_PARAMETER(e);
			wchar_t errorMsgBuffer[256];
			wsprintfW(errorMsgBuffer, L"FUNCTION: Audio::loadCompressedSound()\n\nMESSAGE: Failed to read data chunk!");
			MessageBox(NULL, errorMsgBuffer, static_cast<LPCWSTR>(L"AUDIO ERROR!"), MB_ICONERROR);
			std::exit(0);
		}

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

#pragma region ERROR_CHECKING
		try {
			if (hr != S_OK) {
				throw std::invalid_argument(nullptr);
			}
		}
		catch (const std::invalid_argument& e) {

			UNREFERENCED_PARAMETER(e);
			wchar_t errorMsgBuffer[256];
			wsprintfW(errorMsgBuffer, L"FUNCTION: Audio::loadCompressedSound()\n\nMESSAGE: Failed to find the desired data chunk!");
			MessageBox(NULL, errorMsgBuffer, static_cast<LPCWSTR>(L"AUDIO ERROR!"), MB_ICONERROR);
			std::exit(0);
		}
#pragma endregion

		// reading data from WAV files
		hr = readChunkData(hFile, &m_formatADPCM, dwChunkSize, dwChunkPosition);
		// reading data from ADPC-WAV files (compressed)
		/*hr = ReadChunkData(hFile, &adpcwf, dwChunkSize, dwChunkPosition);*/

#pragma region ERROR_CHECKING
		try {
			if (hr != S_OK) {
				throw std::invalid_argument(nullptr);
			}
		}
		catch (const std::invalid_argument& e) {

			UNREFERENCED_PARAMETER(e);
			wchar_t errorMsgBuffer[256];
			wsprintfW(errorMsgBuffer, L"FUNCTION: Audio::loadCompressedSound()\n\nMESSAGE: Failed to read data chunk!");
			MessageBox(NULL, errorMsgBuffer, static_cast<LPCWSTR>(L"AUDIO ERROR!"), MB_ICONERROR);
			std::exit(0);
		}
#pragma endregion

		hr = findChunk(hFile, fourccDATA, dwChunkSize, dwChunkPosition);

#pragma region ERROR_CHECKING
		try {
			if (hr != S_OK) {
				throw std::invalid_argument(nullptr);
			}
		}
		catch (const std::invalid_argument& e) {

			UNREFERENCED_PARAMETER(e);
			wchar_t errorMsgBuffer[256];
			wsprintfW(errorMsgBuffer, L"FUNCTION: Audio::loadCompressedSound()\n\nMESSAGE: Failed to find the desired data chunk!");
			MessageBox(NULL, errorMsgBuffer, static_cast<LPCWSTR>(L"AUDIO ERROR!"), MB_ICONERROR);
			std::exit(0);
		}
#pragma endregion

		BYTE* pDataBuffer = new BYTE[dwChunkSize];

		hr = readChunkData(hFile, pDataBuffer, dwChunkSize, dwChunkPosition);

#pragma region ERROR_CHECKING
		try {
			if (hr != S_OK) {
				throw std::invalid_argument(nullptr);
			}
		}
		catch (const std::invalid_argument& e) {

			UNREFERENCED_PARAMETER(e);
			wchar_t errorMsgBuffer[256];
			wsprintfW(errorMsgBuffer, L"FUNCTION: Audio::loadCompressedSound()\n\nMESSAGE: Failed to read data chunk!");
			MessageBox(NULL, errorMsgBuffer, static_cast<LPCWSTR>(L"AUDIO ERROR!"), MB_ICONERROR);
			std::exit(0);
		}
#pragma endregion ERROR_CHECKING

		audioData.m_soundBuffer.AudioBytes = dwChunkSize;  //buffer containing audio data
		audioData.m_soundBuffer.pAudioData = pDataBuffer;  //size of the audio buffer in bytes
		audioData.m_soundBuffer.Flags = XAUDIO2_END_OF_STREAM; // tell the source voice not to expect any data after this buffer

		// creating a 'sourceVoice' for WAV file-type
		hr = xAudio2->CreateSourceVoice(&audioData.m_sourceVoice, (WAVEFORMATEX*)& m_formatADPCM);

#pragma region ERROR_CHECKING
		try {
			if (hr != S_OK) {
				throw std::invalid_argument(nullptr);
			}
		}
		catch (const std::invalid_argument& e) {

			UNREFERENCED_PARAMETER(e);
			wchar_t errorMsgBuffer[256];
			wsprintfW(errorMsgBuffer, L"FUNCTION: Audio::loadCompressedSound()\n\nMESSAGE: Failed to create the actual 'SourceVoice' for the sound!");
			MessageBox(NULL, errorMsgBuffer, static_cast<LPCWSTR>(L"AUDIO ERROR!"), MB_ICONERROR);
			std::exit(0);
		}
#pragma endregion

		hr = audioData.m_sourceVoice->SubmitSourceBuffer(&audioData.m_soundBuffer);

#pragma region ERROR_CHECKING
		try {
			if (hr != S_OK) {
				throw std::invalid_argument(nullptr);
			}
		}
		catch (const std::invalid_argument& e) {

			UNREFERENCED_PARAMETER(e);
			wchar_t errorMsgBuffer[256];
			wsprintfW(errorMsgBuffer, L"FUNCTION: Audio::loadCompressedSound()\n\nMESSAGE: Failed submit processed audio data to data buffer!");
			MessageBox(NULL, errorMsgBuffer, static_cast<LPCWSTR>(L"AUDIO ERROR!"), MB_ICONERROR);
			std::exit(0);
		}
#pragma endregion
	}

	std::string WAVLoader::getFileExtension(const std::string& filename) {

		size_t i = filename.rfind('.', filename.length());

		if (i != std::string::npos) {
			return(filename.substr(i + 1, filename.length() - i));
		}

		return("");
	}

	std::wstring WAVLoader::s2ws(const std::string& s)
	{
		int len;
		int slength = (int)s.length() + 1;
		len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
		wchar_t* buf = new wchar_t[len];
		MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
		std::wstring r(buf);
		delete[] buf;
		return r;
	}

	HRESULT WAVLoader::findChunk(HANDLE hFile, DWORD fourcc, DWORD& dwChunkSize, DWORD& dwChunkDataPosition)
	{
		HRESULT hr = S_OK;
		if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, 0, NULL, FILE_BEGIN))
			return HRESULT_FROM_WIN32(GetLastError());

		DWORD dwChunkType;
		DWORD dwChunkDataSize;
		DWORD dwRIFFDataSize = 0;
		DWORD dwFileType;
		DWORD bytesRead = 0;
		DWORD dwOffset = 0;

		while (hr == S_OK)
		{
			DWORD dwRead;
			if (0 == ReadFile(hFile, &dwChunkType, sizeof(DWORD), &dwRead, NULL))
				hr = HRESULT_FROM_WIN32(GetLastError());

			if (0 == ReadFile(hFile, &dwChunkDataSize, sizeof(DWORD), &dwRead, NULL))
				hr = HRESULT_FROM_WIN32(GetLastError());

			switch (dwChunkType)
			{
			case fourccRIFF:
				dwRIFFDataSize = dwChunkDataSize;
				dwChunkDataSize = 4;
				if (0 == ReadFile(hFile, &dwFileType, sizeof(DWORD), &dwRead, NULL))
					hr = HRESULT_FROM_WIN32(GetLastError());
				break;

			default:
				if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, dwChunkDataSize, NULL, FILE_CURRENT))
					return HRESULT_FROM_WIN32(GetLastError());
			}

			dwOffset += sizeof(DWORD) * 2;

			if (dwChunkType == fourcc)
			{
				dwChunkSize = dwChunkDataSize;
				dwChunkDataPosition = dwOffset;
				return S_OK;
			}

			dwOffset += dwChunkDataSize;

			if (bytesRead >= dwRIFFDataSize) return S_FALSE;

		}

		return S_OK;
	}

	HRESULT WAVLoader::readChunkData(HANDLE hFile, void* buffer, DWORD buffersize, DWORD bufferoffset)
	{
		HRESULT hr = S_OK;
		if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, bufferoffset, NULL, FILE_BEGIN))
			return HRESULT_FROM_WIN32(GetLastError());
		DWORD dwRead;
		if (0 == ReadFile(hFile, buffer, buffersize, &dwRead, NULL))
			hr = HRESULT_FROM_WIN32(GetLastError());
		return hr;
	}
}
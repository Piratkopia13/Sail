#include "game/Game.h"

#include "Xaudio2.h"
#define fourccRIFF 'FFIR'
#define fourccDATA 'atad'
#define fourccFMT ' tmf'
#define fourccWAVE 'EVAW'
#define fourccXWMA 'AMWX'
#define fourccDPDS 'sdpd'

std::wstring s2ws(const std::string& s)
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

HRESULT FindChunk(HANDLE hFile, DWORD fourcc, DWORD& dwChunkSize, DWORD& dwChunkDataPosition)
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

HRESULT ReadChunkData(HANDLE hFile, void* buffer, DWORD buffersize, DWORD bufferoffset)
{
	HRESULT hr = S_OK;
	if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, bufferoffset, NULL, FILE_BEGIN))
		return HRESULT_FROM_WIN32(GetLastError());
	DWORD dwRead;
	if (0 == ReadFile(hFile, buffer, buffersize, &dwRead, NULL))
		hr = HRESULT_FROM_WIN32(GetLastError());
	return hr;
}

// Entry point for windows subsystem
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd) {
	
	HRESULT hr;
	hr = CoInitialize(nullptr);

// Show console if compiled in debug
#ifdef _DEBUG
	AllocConsole();
	FILE* a;
	freopen_s(&a, "CONIN$", "r", stdin);
	freopen_s(&a, "CONOUT$", "w", stdout);
	freopen_s(&a, "CONOUT$", "w", stderr);
#endif

	// Check for memory leaks
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);

// ---------------------------------------------------------------------------------------

	IXAudio2* xAudio = nullptr;
	hr = XAudio2Create(&xAudio, 0, XAUDIO2_DEFAULT_PROCESSOR);
	if (hr != S_OK) {
		std::cout << "OH NO, ERROR_1!";
	}
	
	IXAudio2MasteringVoice* pMasterVoice = nullptr;
	hr = xAudio->CreateMasteringVoice(&pMasterVoice);
	if (hr != S_OK) {
		std::cout << "OH NO, ERROR_1!";
	}

	//WAVEFORMATEXTENSIBLE wfx = { 0 };
	ADPCMWAVEFORMAT adpcwf = { 0 };
	XAUDIO2_BUFFER buffer = { 0 };

	std::string strFileName = "../Audio/sample3.wav";
	std::wstring wstrFileName = s2ws(strFileName);
	LPCWSTR fileName = wstrFileName.c_str();

	HANDLE hFile = CreateFile(
		fileName,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		0,
		nullptr);

	if (INVALID_HANDLE_VALUE == hFile) {
		return HRESULT_FROM_WIN32(GetLastError());
	}

	if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, 0, NULL, FILE_BEGIN)) {
		return HRESULT_FROM_WIN32(GetLastError());
	}

	DWORD dwChunkSize;
	DWORD dwChunkPosition;
	//check the file type, should be fourccWAVE or 'XWMA'
	
	hr = FindChunk(hFile, fourccRIFF, dwChunkSize, dwChunkPosition);
	if (hr != S_OK) {
		std::cout << "OH NO, ERROR_1!";
	}

	DWORD filetype;
	hr = ReadChunkData(hFile, &filetype, sizeof(DWORD), dwChunkPosition);
	if (hr != S_OK) {
		std::cout << "OH NO, ERROR_1!";
	}

	if (filetype != fourccWAVE) {
		return S_FALSE;
	}

	hr = FindChunk(hFile, fourccFMT, dwChunkSize, dwChunkPosition);
	if (hr != S_OK) {
		std::cout << "OH NO, ERROR_1!";
	}

	hr = ReadChunkData(hFile, &adpcwf, dwChunkSize, dwChunkPosition);
	if (hr != S_OK) {
		std::cout << "OH NO, ERROR_1!";
	}

	hr = FindChunk(hFile, fourccDATA, dwChunkSize, dwChunkPosition);
	if (hr != S_OK) {
		std::cout << "OH NO, ERROR_1!";
	}

	BYTE* pDataBuffer = new BYTE[dwChunkSize];
	
	hr = ReadChunkData(hFile, pDataBuffer, dwChunkSize, dwChunkPosition);
	if (hr != S_OK) {
		std::cout << "OH NO, ERROR_1!";
	}

	buffer.AudioBytes = dwChunkSize;  //buffer containing audio data
	buffer.pAudioData = pDataBuffer;  //size of the audio buffer in bytes
	buffer.Flags = XAUDIO2_END_OF_STREAM; // tell the source voice not to expect any data after this buffer

	IXAudio2SourceVoice* pSourceVoice;
	hr = xAudio->CreateSourceVoice(&pSourceVoice, (WAVEFORMATEX*)& adpcwf);
	if (hr != S_OK) {
		std::cout << "OH NO, ERROR_1!";
	}

	hr = pSourceVoice->SubmitSourceBuffer(&buffer);
	if (hr != S_OK) {
		std::cout << "OH NO, ERROR_2!";
	}

	hr = pSourceVoice->Start(0);
	if (hr != S_OK) {
		std::cout << "OH NO, ERROR_3!";
	}

// ---------------------------------------------------------------------------------------

	Game game(hInstance);
	game.run();

	//OutputDebugString(L"\n========= Memory leak report =========\n");
	//_CrtDumpMemoryLeaks();
	//OutputDebugString(L"======================================\n\n");

	return 0;
}
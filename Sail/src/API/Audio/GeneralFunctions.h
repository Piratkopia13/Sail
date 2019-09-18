#ifndef GENERAL_FUNCTIONS_H
#define GENERAL_FUNCTIONS_H

//#define min(a, b)  (((a) < (b)) ? (a) : (b))

#define fourccRIFF 'FFIR'
#define fourccDATA 'atad'
#define fourccFMT  ' tmf'
#define fourccWAVE 'EVAW'
#define fourccXWMA 'AMWX'
#define fourccDPDS 'sdpd'

//#include <string>
#include <comdef.h> 
static std::string getFileExtension(const std::string& filename) {
	size_t i = filename.rfind('.', filename.length());

	if (i != std::string::npos) {
		return(filename.substr(i + 1, filename.length() - i));
	}

	return("");
}

static std::wstring stringToWString(const std::string& s) {

	std::wstring returnString = std::wstring(s.begin(), s.end());
	return returnString;
}

static HRESULT findChunk(HANDLE hFile, DWORD fourcc, DWORD& dwChunkSize, DWORD& dwChunkDataPosition)
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

static HRESULT readChunkData(HANDLE hFile, void* buffer, DWORD buffersize, DWORD bufferoffset) {
	HRESULT hr = S_OK;
	if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, bufferoffset, NULL, FILE_BEGIN))
		return HRESULT_FROM_WIN32(GetLastError());
	DWORD dwRead;
	if (0 == ReadFile(hFile, buffer, buffersize, &dwRead, NULL))
		hr = HRESULT_FROM_WIN32(GetLastError());
	return hr;
}

static void errorCheck(HRESULT hr, std::string titleWindow, std::string titleMessage, std::string message, int errorType = 0, bool exitIfFailed = true) {

	wchar_t convertedMessage[MAX_PATH];
	wchar_t convertedTitle[MAX_PATH];
	wsprintfW(convertedMessage, static_cast<LPCWSTR>(stringToWString(titleMessage + "\n\nMESSAGE: " + message).c_str()));
	wsprintfW(convertedTitle, static_cast<LPCWSTR>(stringToWString(titleWindow).c_str()));

	long errorCode = 0;
	
	if (errorType == 0) {
		errorCode = MB_ICONERROR;
	}
	else if (errorType == 1) {
		errorCode = MB_ICONWARNING;
	}
	else if (errorType == 2) {
		errorCode = MB_ICONEXCLAMATION;
	}

	try {
		if (hr != S_OK) {
			throw std::invalid_argument(nullptr);
		}
	}
	catch (const std::invalid_argument& e) {

		UNREFERENCED_PARAMETER(e);
		wchar_t errorMsgBuffer[256];
		wsprintfW(errorMsgBuffer, static_cast<LPCWSTR>(convertedMessage));
		MessageBox(NULL, errorMsgBuffer, static_cast<LPCWSTR>(convertedTitle), errorType);
		if (exitIfFailed) {
			std::exit(0);
		}
	}
}

#endif
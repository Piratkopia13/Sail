#include "pch.h"
#include "SailGuiWindow.h"

#include <commdlg.h>
#include <atlstr.h>

std::string SailGuiWindow::openFileDialog(LPCWSTR filter /*= L"All Files (*.*)\0*.*\0"*/, HWND owner /*= NULL*/) {
	OPENFILENAME ofn;
	WCHAR fileName[MAX_PATH] = L"";
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = owner;
	ofn.lpstrFilter = filter;
	ofn.lpstrFile = fileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_NOCHANGEDIR;
	ofn.lpstrDefExt = L"";
	std::string fileNameStr;
	if (GetOpenFileName(&ofn))
		fileNameStr = CW2A(fileName);
	return fileNameStr;
}

void SailGuiWindow::limitStringLength(std::string& str, int maxLength /*= 20*/) {
	if (str.length() > maxLength) {
		str = "..." + str.substr(str.length() - maxLength + 3, maxLength - 3);
	}
}

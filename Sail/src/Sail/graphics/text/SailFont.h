#pragma once

#include <string>
//#include <SpriteFont.h>

namespace {
	static const std::wstring DEFAULT_SPRITEFONT_LOCATION = L"res/fonts/";
}

class SailFont {

public:
	SailFont(LPCWSTR fontFilename = L"courierNew.spritefont");
	~SailFont();

	//DirectX::SpriteFont* get();

private:
	//std::unique_ptr<DirectX::SpriteFont> m_font;
	

};
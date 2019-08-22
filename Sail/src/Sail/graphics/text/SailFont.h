#pragma once

#include <string>
//#include <SpriteFont.h>

class SailFont {
public:
	static const std::wstring DEFAULT_SPRITEFONT_LOCATION;

public:
	SailFont(LPCWSTR fontFilename = L"courierNew.spritefont");
	~SailFont();

	//DirectX::SpriteFont* get();

private:
	//std::unique_ptr<DirectX::SpriteFont> m_font;
	

};
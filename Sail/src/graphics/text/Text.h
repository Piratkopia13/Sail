#pragma once

#include <d3d11.h>
#include <SimpleMath.h>
#include <SpriteBatch.h>

class SailFont;

class Text {

public:
	Text(SailFont* font, const std::wstring& text, const DirectX::SimpleMath::Vector2& pos = DirectX::SimpleMath::Vector2::Zero, const DirectX::SimpleMath::Vector4& color = DirectX::SimpleMath::Vector4::One);

	void setText(const std::wstring& text);
	void setPosition(const DirectX::SimpleMath::Vector2& pos);
	void draw(DirectX::SpriteBatch* batch);

private:
	SailFont* m_font;
	std::wstring m_text;
	DirectX::SimpleMath::Vector2 m_pos;
	DirectX::SimpleMath::Vector4 m_color;

};
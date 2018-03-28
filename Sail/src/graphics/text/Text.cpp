#include "Text.h"

#include "SailFont.h"
#include "../shader/ShaderSet.h"

using namespace DirectX::SimpleMath;

Text::Text(SailFont* font, const std::wstring& text, const Vector2& pos, const Vector4& color)
	: m_font(font)
	, m_text(text)
	, m_pos(pos)
	, m_color(color)
{	}

void Text::setText(const std::wstring& text) {
	m_text = text;
}
void Text::setPosition(const Vector2& pos) {
	m_pos = pos;
}

void Text::draw(DirectX::SpriteBatch* batch) {
	m_font->get()->DrawString(batch, m_text.c_str(), m_pos, m_color);
	ShaderSet::CurrentlyBoundShader = nullptr;
}
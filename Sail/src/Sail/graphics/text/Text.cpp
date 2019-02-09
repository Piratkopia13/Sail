#include "pch.h"
#include "Text.h"

#include "SailFont.h"
#include "../shader/ShaderSet.h"

using namespace DirectX::SimpleMath;

Text::Ptr Text::Create(const std::wstring& text, const Vector2& pos, const Vector4& color) {
	return std::make_unique<Text>(text, pos, color);
}

Text::Text(const std::wstring& text, const Vector2& pos, const Vector4& color)
	: m_text(text)
	, m_pos(pos)
	, m_color(color)
{	}

void Text::setText(const std::wstring& text) {
	m_text = text;
}

const std::wstring& Text::getText() const {
	return m_text;
}

void Text::setPosition(const Vector2& pos) {
	m_pos = pos;
}

const Vector2& Text::getPosition() const {
	return m_pos;
}

void Text::setColor(const Vector4& color) {
	m_color = color;
}

const Vector4& Text::getColor() const {
	return m_color;
}

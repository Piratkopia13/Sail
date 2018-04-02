#pragma once

#include <d3d11.h>
#include <SimpleMath.h>
#include <SpriteBatch.h>

class SailFont;

class Text {
public:
	typedef std::unique_ptr<Text> Ptr;
	static Ptr Create(const std::wstring& text, const DirectX::SimpleMath::Vector2& pos = DirectX::SimpleMath::Vector2::Zero, const DirectX::SimpleMath::Vector4& color = DirectX::SimpleMath::Vector4::One);
public:
	Text(const std::wstring& text, const DirectX::SimpleMath::Vector2& pos = DirectX::SimpleMath::Vector2::Zero, const DirectX::SimpleMath::Vector4& color = DirectX::SimpleMath::Vector4::One);

	void setText(const std::wstring& text);
	const std::wstring& getText() const;
	void setPosition(const DirectX::SimpleMath::Vector2& pos);
	const DirectX::SimpleMath::Vector2& getPosition() const;
	void setColor(const DirectX::SimpleMath::Vector4& color);
	const DirectX::SimpleMath::Vector4& getColor() const;

private:
	std::wstring m_text;
	DirectX::SimpleMath::Vector2 m_pos;
	DirectX::SimpleMath::Vector4 m_color;

};
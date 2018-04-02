#pragma once

#include "Component.h"
#include "../../graphics/text/SailFont.h"
#include "../../graphics/text/Text.h"

class TextComponent : public Component/*, public IDrawable*/ {
public:
	SAIL_COMPONENT
	TextComponent();
	~TextComponent();

	Text* addText(Text::Ptr text);

	void draw()/* override*/;

private:
	SailFont m_font;
	std::vector<Text::Ptr> m_texts;
	std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;

};


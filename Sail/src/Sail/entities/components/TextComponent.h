#pragma once

#include <vector>
#include "Component.h"
#include "../../graphics/text/SailFont.h"
#include "../../graphics/text/Text.h"

class TextComponent : public Component<TextComponent> /*, public IDrawable*/ {
public:
	TextComponent();
	~TextComponent();

	Text* addText(Text::Ptr text);

	void draw()/* override*/;

#ifdef DEVELOPMENT
	const unsigned int getByteSize() const override {
		return sizeof(*this) + sizeof(Text) * m_texts.size();
	}
#endif

private:
	SailFont m_font;
	std::vector<Text::Ptr> m_texts;
	//std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;

};


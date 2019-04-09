#pragma once

#include <SimpleMath.h>

class RenderableTexture {
public:
	RenderableTexture(unsigned int width = 320, unsigned int height = 180);
	virtual ~RenderableTexture();

	virtual void begin() = 0;
	virtual void end() = 0;
	virtual void clear(const DirectX::XMVECTORF32& color) = 0;
	virtual void resize(int width, int height) = 0;

protected:

private:

};
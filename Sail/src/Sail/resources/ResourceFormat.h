#pragma once

namespace ResourceFormat {

	struct TextureData {
		unsigned int width;
		unsigned int height;
		unsigned int channels;
		unsigned char* textureData;
	};

}

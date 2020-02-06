#pragma once

namespace ResourceFormat {

	enum TextureFormat {
		R8,
		R8G8,
		R8G8B8A8,
		R16_FLOAT,
		R16G16_FLOAT,
		R16G16B16A16_FLOAT,
		R32G32B32A32_FLOAT
	};

	struct TextureData {
		unsigned int width;
		unsigned int height;
		unsigned int bitsPerChannel;
		unsigned int channels;
		float* textureDataFloat = nullptr;
		unsigned char* textureData8bit = nullptr;
		TextureFormat format = R8G8B8A8;
	};

}

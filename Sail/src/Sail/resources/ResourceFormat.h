#pragma once

namespace ResourceFormat {

	enum TextureFormat {
		R8,
		R8G8,
		R8G8B8A8,
		R16_FLOAT,
		R16G16_FLOAT,
		R16G16B16A16_FLOAT,
		R32G32B32A32_FLOAT,
		BC3,
		BC5,
		BC7,
	};

	struct TextureData {
		unsigned int width;
		unsigned int height;
		unsigned int bitsPerChannel;
		unsigned int channels;
		unsigned int byteSize;
		void* data = nullptr;
		bool isCubeMap = false;
		bool isSRGB = false;
		TextureFormat format = R8G8B8A8;
	};

}

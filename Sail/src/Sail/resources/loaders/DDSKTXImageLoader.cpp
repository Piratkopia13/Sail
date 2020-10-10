#include "pch.h"
#include "DDSKTXImageLoader.h"
#include "Sail/utils/Utils.h"

#define DDSKTX_IMPLEMENT
#include "dds-ktx/dds-ktx.h"

FileLoader::DDSKTXImageLoader::DDSKTXImageLoader(const std::string& filename, ResourceFormat::TextureData& textureData) {
	auto ddsData = Utils::readFileBinary(filename);

	int size = ddsData.size();

	assert(!ddsData.empty());
	ddsktx_texture_info tc = { 0 };
	ddsktx_error err;
	
	if (ddsktx_parse(&tc, ddsData.data(), size, &err)) {
		assert(tc.depth == 1);
		assert(tc.num_layers == 1);
		
		// Load each mip level into an offset
		textureData.data = SAIL_NEW std::byte[tc.size_bytes];

		unsigned int offset = 0;
		for (int mip = 0; mip < tc.num_mips; mip++) {
			for (int face = 0; face < DDSKTX_CUBE_FACE_COUNT; face++) {
				ddsktx_sub_data subData;
				ddsktx_get_sub(&tc, &subData, ddsData.data(), size, 0, face, mip);

				if (face == 0) {
					textureData.mipExtents.emplace_back(subData.width, subData.height);
					textureData.mipOffsets.emplace_back(offset);
				}

				memcpy((char*)textureData.data + offset, subData.buff, subData.size_bytes);
				offset += subData.size_bytes;
			}
		}
		textureData.byteSize = offset;
		
			
		/*textureData.byteSize = tc.size_bytes;
		textureData.data = SAIL_NEW std::byte[textureData.byteSize];
		memcpy(textureData.data, &ddsData[tc.data_offset], textureData.byteSize);*/
		textureData.width = tc.width;
		textureData.height = tc.height;

		textureData.mipLevels = tc.num_mips;
		textureData.format = convertFormat(tc.format);
		textureData.isCubeMap = (tc.flags & DDSKTX_TEXTURE_FLAG_CUBEMAP);
		textureData.isSRGB = (tc.flags & DDSKTX_TEXTURE_FLAG_SRGB);
		textureData.bitsPerChannel = tc.bpp;

		textureData.channels = 4; // TODO: make this right

	} else {
		Logger::Error(err.msg);
	}
}

FileLoader::DDSKTXImageLoader::~DDSKTXImageLoader() { }

ResourceFormat::TextureFormat FileLoader::DDSKTXImageLoader::convertFormat(ddsktx_format ddsktxFormat) const {
	switch (ddsktxFormat) {
	case DDSKTX_FORMAT_BC1:
		break;
	case DDSKTX_FORMAT_BC2:
		break;
	case DDSKTX_FORMAT_BC3:
		return ResourceFormat::BC3;
		break;
	case DDSKTX_FORMAT_BC4:
		break;
	case DDSKTX_FORMAT_BC5:
		return ResourceFormat::BC5;
		break;
	case DDSKTX_FORMAT_BC6H:
		break;
	case DDSKTX_FORMAT_BC7:
		return ResourceFormat::BC7;
		break;
	case DDSKTX_FORMAT_ETC1:
		break;
	case DDSKTX_FORMAT_ETC2:
		break;
	case DDSKTX_FORMAT_ETC2A:
		break;
	case DDSKTX_FORMAT_ETC2A1:
		break;
	case DDSKTX_FORMAT_PTC12:
		break;
	case DDSKTX_FORMAT_PTC14:
		break;
	case DDSKTX_FORMAT_PTC12A:
		break;
	case DDSKTX_FORMAT_PTC14A:
		break;
	case DDSKTX_FORMAT_PTC22:
		break;
	case DDSKTX_FORMAT_PTC24:
		break;
	case DDSKTX_FORMAT_ATC:
		break;
	case DDSKTX_FORMAT_ATCE:
		break;
	case DDSKTX_FORMAT_ATCI:
		break;
	case DDSKTX_FORMAT_ASTC4x4:
		break;
	case DDSKTX_FORMAT_ASTC5x5:
		break;
	case DDSKTX_FORMAT_ASTC6x6:
		break;
	case DDSKTX_FORMAT_ASTC8x5:
		break;
	case DDSKTX_FORMAT_ASTC8x6:
		break;
	case DDSKTX_FORMAT_ASTC10x5:
		break;
	case _DDSKTX_FORMAT_COMPRESSED:
		break;
	case DDSKTX_FORMAT_A8:
		break;
	case DDSKTX_FORMAT_R8:
		return ResourceFormat::R8;
		break;
	case DDSKTX_FORMAT_RGBA8:
		return ResourceFormat::R8G8B8A8;
		break;
	case DDSKTX_FORMAT_RGBA8S:
		break;
	case DDSKTX_FORMAT_RG16:
		break;
	case DDSKTX_FORMAT_RGB8:
		break;
	case DDSKTX_FORMAT_R16:
		break;
	case DDSKTX_FORMAT_R32F:
		break;
	case DDSKTX_FORMAT_R16F:
		return ResourceFormat::R16_FLOAT;
		break;
	case DDSKTX_FORMAT_RG16F:
		return ResourceFormat::R16G16_FLOAT;
		break;
	case DDSKTX_FORMAT_RG16S:
		break;
	case DDSKTX_FORMAT_RGBA16F:
		return ResourceFormat::R16G16B16A16_FLOAT;
		break;
	case DDSKTX_FORMAT_RGBA16:
		break;
	case DDSKTX_FORMAT_BGRA8:
		break;
	case DDSKTX_FORMAT_RGB10A2:
		break;
	case DDSKTX_FORMAT_RG11B10F:
		break;
	case DDSKTX_FORMAT_RG8:
		return ResourceFormat::R8G8;
		break;
	case DDSKTX_FORMAT_RG8S:
		break;
	case _DDSKTX_FORMAT_COUNT:
		break;
	default:
		break;
	}
	assert(false && "Format missing from convert method");
}

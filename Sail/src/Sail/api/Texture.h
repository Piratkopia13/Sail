#pragma once

#include <string.h>
#include "Sail/resources/TextureData.h"

class Texture {
public:
	enum ADDRESS_MODE {
		WRAP,
		MIRROR,
		CLAMP,
		BORDER,
		MIRROR_ONCE
	};
	enum FILTER {
		LINEAR,
		POINT,
		ANISOTROPIC,
		// TODO: add more filters if needed
	};
	
public:
	static Texture* Create(const std::string& filename, bool useAbsolutePath = false);
	Texture(const std::string& filename);
	virtual ~Texture() {}

	const std::string& getName() const;

protected:
	TextureData& getTextureData(const std::string& filename, bool useAbsolutePath) const;

private:
	std::string m_name;

};
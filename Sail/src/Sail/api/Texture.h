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
	static Texture* Create(const std::string& filepath);
	Texture(const std::string& filepath);
	virtual ~Texture() {}

	const std::string& getName() const;
	bool isCubeMap() const;
	bool isReadyToUse() const; // Returns true when texture is ready for usage

protected:
	TextureData& getTextureData(const std::string& filepath) const;

protected:
	bool readyToUse;
	bool texIsCubeMap;

private:
	std::string m_name;

};
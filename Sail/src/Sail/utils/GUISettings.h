#pragma once

#include <string>
#include <glm/glm.hpp>

namespace GUIText {
	const std::string fontTexture = "fonts/minecraft-font-character-map-v2.tga";
	const int numX = 9;
	const int numY = 3;
	const int fontMapSizeX = 990; // pixels
	const int fontMapSizeY = 361; // pixels
	const glm::vec2 charSize = glm::vec2((static_cast<float>(fontMapSizeX) / static_cast<float>(numX)) / static_cast<float>(fontMapSizeX),
										 (static_cast<float>(fontMapSizeY) / static_cast<float>(numY)) / static_cast<float>(fontMapSizeY));
	const int spacePosX = 8;
	const int spacePosY = 2;
}
#pragma once

#include <string>
#include <glm/glm.hpp>

namespace GUIText {
	const std::string fontTexture = "fonts/minecraft-font-character-map-v2.tga";
	const int numX = 9; // Num characters horizontally
	const int numY = 3; // Num characters vertically
	const int fontMapSizeX = 990; // horizontal pixel size of spritemap
	const int fontMapSizeY = 361; // vertical...
	const int spacePosX = 8; // Horizontal index of ' ' in the spritemap
	const int spacePosY = 2; // Vertical index of ' ' in the spritemap
	const glm::vec2 charSize = glm::vec2((static_cast<float>(fontMapSizeX) / static_cast<float>(numX)) / static_cast<float>(fontMapSizeX),
		(static_cast<float>(fontMapSizeY) / static_cast<float>(numY)) / static_cast<float>(fontMapSizeY));
}
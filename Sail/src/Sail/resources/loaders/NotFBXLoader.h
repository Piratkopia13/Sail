#pragma once

#include "Sail/graphics/geometry/Model.h"
#include "Sail/graphics/geometry/Animation.h"
#include <string>

namespace NotFBXLoader {

	void Load(std::string filename, Model*& model, Shader* shader, AnimationStack*& animationStack);
	void Save(std::string filename, Model* model, AnimationStack* animationStack);

}
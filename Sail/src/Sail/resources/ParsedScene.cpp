#include "pch.h"
#include "ParsedScene.h"
#include "../graphics/geometry/Model.h"
#include "../utils/Utils.h"

ParsedScene::ParsedScene(const std::string& filename, ShaderSet* shaderSet) {

	if (filename.substr(filename.size() - 3) != "fbx") {
		Logger::Error("Only FBX models are currently supported! Tried to load \"" + filename + "\"");
	}

	FBXLoader loader(DEFAULT_MODEL_LOCATION + filename, shaderSet);
	m_model = std::move(loader.getModel());
}

ParsedScene::~ParsedScene() {
}

Model* ParsedScene::getModel() {
	return m_model.get();
}
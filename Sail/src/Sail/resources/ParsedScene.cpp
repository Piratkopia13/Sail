#include "pch.h"
#include "ParsedScene.h"
#include "../graphics/geometry/Model.h"
#include "../utils/Utils.h"

const std::string ParsedScene::DEFAULT_MODEL_LOCATION = "res/models/";

ParsedScene::ParsedScene(const std::string& filename, Shader* shader, bool useAbsolutePath) {

	if (filename.substr(filename.size() - 3) != "fbx") {
		Logger::Error("Only FBX models are currently supported! Tried to load \"" + filename + "\"");
	}

	std::string filepath = (useAbsolutePath) ? filename : DEFAULT_MODEL_LOCATION + filename;
	FBXLoader loader(filepath, shader);
	m_model = std::move(loader.getModel());
}

ParsedScene::~ParsedScene() {
}

Model* ParsedScene::getModel() {
	return m_model.get();
}
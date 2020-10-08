#include "pch.h"

#include "loaders/FBXLoader.h"
#include "ParsedScene.h"
#include "../graphics/geometry/Model.h"
#include "../utils/Utils.h"

const std::string ParsedScene::DEFAULT_MODEL_LOCATION = "res/models/";

ParsedScene::ParsedScene(const std::string& filename, bool useAbsolutePath) {

	std::string extension = filename.substr(filename.size() - 3);
	std::transform(extension.begin(), extension.end(), extension.begin(), [](unsigned char c) { return std::tolower(c); }); // To lower case
	if (extension != "fbx") {
		Logger::Error("Only FBX models are currently supported! Tried to load \"" + filename + "\"");
	}

	std::string filepath = (useAbsolutePath) ? filename : DEFAULT_MODEL_LOCATION + filename;
	FBXLoader loader(filepath);
	m_model = loader.getModel();
}

ParsedScene::~ParsedScene() {
}

std::shared_ptr<Model> ParsedScene::getModel() {
	return m_model;
}
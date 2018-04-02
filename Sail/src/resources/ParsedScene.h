#pragma once

#include "loaders/FBXLoader.h"

class Model;

namespace {
	static const std::string DEFAULT_MODEL_LOCATION = "res/models/";
}

class ParsedScene {
public:
	ParsedScene(const std::string& filename, ShaderSet* shaderSet);

	~ParsedScene();
	Model* getModel();

private:
	std::unique_ptr<Model> m_model;

};
#pragma once

#include "loaders/FBXLoader.h"

class Model;

class ParsedScene {
public:
	static const std::string DEFAULT_MODEL_LOCATION;

public:
	ParsedScene(const std::string& filename, Shader* shader, bool useAbsolutePath = false);

	~ParsedScene();
	std::shared_ptr<Model> getModel();

private:
	std::shared_ptr<Model> m_model;

};
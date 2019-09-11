#pragma once

#include "loaders/FBXLoader.h"

class Model;

class ParsedScene {
public:
	static const std::string DEFAULT_MODEL_LOCATION;

public:
	ParsedScene(const std::string& filename, Shader* shader);

	~ParsedScene();
	Model* getModel();

private:
	std::unique_ptr<Model> m_model;

};
#pragma once

class Model;

class ParsedScene {
public:
	static const std::string DEFAULT_MODEL_LOCATION;

public:
	ParsedScene(const std::string& filename, bool useAbsolutePath = false);

	~ParsedScene();
	std::shared_ptr<Model> getModel();

private:
	std::shared_ptr<Model> m_model;

};
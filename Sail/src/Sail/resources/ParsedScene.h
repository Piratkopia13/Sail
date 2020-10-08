#pragma once

class Mesh;

class ParsedScene {
public:
	static const std::string DEFAULT_MODEL_LOCATION;

public:
	ParsedScene(const std::string& filename, bool useAbsolutePath = false);

	~ParsedScene();
	std::shared_ptr<Mesh> getMesh();

private:
	std::shared_ptr<Mesh> m_mesh;

};
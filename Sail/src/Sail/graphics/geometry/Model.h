#pragma once

#include <d3d11.h>
#include <vector>
#include "Sail/api/Mesh.h"
#include "Sail/api/Renderer.h"
#include "Sail/utils/Utils.h"

// Forward declarations
class ShaderPipeline;
class Material;

class Model {
public:
	typedef std::unique_ptr<Model> Ptr;

public: 
	Model();
	Model(Mesh::Data& data, Shader* shader);
	~Model();

	Mesh* addMesh(std::unique_ptr<Mesh> mesh);

	// Draws the model using its material
	void draw(const Renderer& renderer);

	Mesh* getMesh(unsigned int index);
	UINT getNumberOfMeshes() const;

	void setIsAnimated(bool animated);
	bool isAnimated() const;

private:
	std::vector<Mesh::Ptr> m_meshes;
	bool m_isAnimated;

};

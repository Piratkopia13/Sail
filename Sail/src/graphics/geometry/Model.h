#pragma once

#include <d3d11.h>
#include <SimpleMath.h>
#include <limits>
#include "../../utils/Utils.h"
#include "Transform.h"
#include "spatial/AABB.h"
#include "../renderer/Renderer.h"
#include "Mesh.h"
#include "Material.h"

// Forward declarations
class ShaderSet;

class Model {
public: 
	Model(Mesh::Data& data, ShaderSet* shaderSet);
	Model(const std::string& path, ShaderSet* shaderSet);
	~Model();

	//void setBuildData(Data& buildData);
	//void buildBufferForShader(ShaderSet* shader);

	// Draws the model using its material
	void draw(Renderer& renderer);

	ShaderSet* getShader() const;
	Material* getMaterial();
	//const AABB& getAABB() const;
	//void updateAABB();

private:
	//void calculateAABB();

private:
	Mesh::SPtr m_mesh;

	Material::SPtr m_material;

	//AABB m_aabb;

};

#pragma once

#include <d3d11.h>
#include <SimpleMath.h>
#include <vector>
#include "../renderer/Renderer.h"
#include "Sail/api/Mesh.h"

// Forward declarations
class ShaderSet;
class Material;

class Model {
public:
	typedef std::unique_ptr<Model> Ptr;

public: 
	//Model(std::vector<Mesh::Data>& data, ShaderSet* shaderSet);
	Model();
	Model(Mesh::Data& data, ShaderSet* shaderSet);
	//Model(const std::string& path, ShaderSet* shaderSet);
	~Model();

	Mesh* addMesh(std::unique_ptr<Mesh> mesh);

	//void setBuildData(Data& buildData);
	//void buildBufferForShader(ShaderSet* shader);

	// Draws the model using its material
	void draw(const Renderer& renderer);

	Mesh* getMesh(unsigned int index);
	unsigned int getNumberOfMeshes() const;
	/*ShaderSet* getShader() const;
	Material* getMaterial();*/
	//const AABB& getAABB() const;
	//void updateAABB();

private:
	//void calculateAABB();

private:
	std::vector<Mesh::Ptr> m_meshes;

	//Material::SPtr m_material;

	//AABB m_aabb;

};

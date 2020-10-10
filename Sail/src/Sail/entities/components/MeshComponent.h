#pragma once

#include "Component.h"
#include "Sail/api/Mesh.h"

class MeshComponent : public Component {
public:
	SAIL_COMPONENT
	MeshComponent(Mesh::SPtr mesh);
	~MeshComponent() { }

	Mesh* get();

	void renderEditorGui(SailGuiWindow* window) override;

private:
	Mesh::SPtr m_mesh;
};
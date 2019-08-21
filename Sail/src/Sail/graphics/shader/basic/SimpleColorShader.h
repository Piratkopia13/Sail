#pragma once

#include <d3d11.h>
#include <glm/glm.hpp>
#include "../ShaderSet.h"
#include "../component/ConstantBuffer.h"
#include "../../geometry/Model.h"
#include "../../geometry/Material.h"
#include "Sail/Application.h"


class SimpleColorShader : public ShaderSet {
public:
	SimpleColorShader();
	~SimpleColorShader();

	void bind() override;

	virtual void draw(Model& model, bool bindFirst = true);

	virtual void createBufferFromModelData(ID3D11Buffer** vertexBuffer, ID3D11Buffer** indexBuffer, ID3D11Buffer** instanceBuffer, const void* data);

	virtual void updateCamera(Camera& cam);

	struct Vertex {
		glm::vec3 position;
		glm::vec4 color;
	};


private:
	void updateBuffer(const glm::vec4& color, const glm::mat4& mvp) const;

private:
	// Input element description
	static D3D11_INPUT_ELEMENT_DESC IED[2];
	// Input layout
	ID3D11InputLayout* m_inputLayout;

	struct ModelDataBuffer {
		glm::vec4 modelColor;
		glm::mat4 mModelViewProj;
	};
	glm::mat4 m_vpMatrix;

	// Components

	// Transform constant buffer structure
	std::unique_ptr<ShaderComponent::ConstantBuffer> m_transformBuffer;

};

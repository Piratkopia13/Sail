#include "pch.h"
#include "DeferredPointLightShader.h"
#include "../../../api/Application.h"


using namespace DirectX;
using namespace SimpleMath;

DeferredPointLightShader::DeferredPointLightShader() 
	: ShaderSet("deferred/PointLightShader.hlsl")
{
	// Create the input layout
	inputLayout.push<Vector3>(InputLayout::POSITION, "POSITION", 0);
	inputLayout.create(VSBlob);

}
DeferredPointLightShader::~DeferredPointLightShader() {
}

void DeferredPointLightShader::setLight(const PointLight& pl, Camera* camera) {

	LightDataBuffer data;
	data.color = pl.getColor();
	data.attCostant = pl.getAttenuation().constant;
	data.attLinear = pl.getAttenuation().linear;
	data.attQuadratic = pl.getAttenuation().quadratic;
	// Set position in view space
	data.positionVS = Vector3::Transform(pl.getPosition(), camera->getViewMatrix());

	setCBufferVar("def_pointLightInput", &data, sizeof(data));

}


void DeferredPointLightShader::bind() {

	// Call parent to bind shaders
	ShaderSet::bind();

}
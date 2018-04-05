#include "DeferredDirectionalLightShader.h"
#include "../../renderer/DeferredRenderer.h"

using namespace DirectX;
using namespace SimpleMath;

DeferredDirectionalLightShader::DeferredDirectionalLightShader() 
	: ShaderSet("deferred/DirectionalLightShader.hlsl")
{

	//// Set up constant buffers
	//ModelDataBuffer defaultModelData = { Matrix::Identity };
	//m_modelDataBuffer = std::unique_ptr<ShaderComponent::ConstantBuffer>(new ShaderComponent::ConstantBuffer(&defaultModelData, sizeof(ModelDataBuffer)));
	//LightDataBuffer defaultLightData = { Vector3::Zero, 0.f, Vector3::Zero };
	//m_lightDataBuffer = std::unique_ptr<ShaderComponent::ConstantBuffer>(new ShaderComponent::ConstantBuffer(&defaultLightData, sizeof(LightDataBuffer)));
	//ShadowLightBuffer defaultShadowLightData = { Matrix::Identity, Matrix::Identity, Matrix::Identity };
	//m_shadowLightBuffer = std::unique_ptr<ShaderComponent::ConstantBuffer>(new ShaderComponent::ConstantBuffer(&defaultShadowLightData, sizeof(ShadowLightBuffer)));

	//// Set up sampler for point sampling
	//m_sampler = std::make_unique<ShaderComponent::Sampler>(D3D11_TEXTURE_ADDRESS_WRAP, D3D11_FILTER_MIN_MAG_MIP_POINT);
	//m_shadowSampler = std::make_unique<ShaderComponent::Sampler>(D3D11_TEXTURE_ADDRESS_WRAP, D3D11_FILTER_MIN_MAG_MIP_POINT);

	// Create the input layout
	inputLayout.push<Vector3>(InputLayout::POSITION, "POSITION", 0);
	inputLayout.create(VSBlob);

}
DeferredDirectionalLightShader::~DeferredDirectionalLightShader() {
}
//
//void DeferredDirectionalLightShader::setLight(const DirectionalLight& dl) {
//	LightDataBuffer data;
//	// Transform the vector into view space using the cameras view matrix
//	// Using TransformNormal since this is a direction, translations should not be applied
//	data.direction = Vector3::TransformNormal(dl.direction, m_mV);
//	data.color = dl.color;
//	m_lightDataBuffer->updateData(&data, sizeof(data));
//}
//
//void DeferredDirectionalLightShader::updateCameraBuffer(Camera& cam, Camera& dlCam) {
//	ShadowLightBuffer data;
//
//	// The inverted view matrix to transform from the player camera's view space to world space
//	data.mInvV = cam.getViewMatrix().Transpose().Invert();
//	data.mLightV = dlCam.getViewMatrix().Transpose();
//	data.mLightP = dlCam.getProjMatrix().Transpose();
//
//	m_shadowLightBuffer->updateData(&data, sizeof(data));
//}

void DeferredDirectionalLightShader::bind() {

	// Call parent to bind shaders
	ShaderSet::bind();

	//// Set input layout as active
	//Application::getInstance()->getDXManager()->getDeviceContext()->IASetInputLayout(m_inputLayout);

	//// Bind cbuffers
	//m_modelDataBuffer->bind(ShaderComponent::VS, 0);
	//m_lightDataBuffer->bind(ShaderComponent::PS, 0);
	//m_shadowLightBuffer->bind(ShaderComponent::PS, 1);

	//// Bind sampler
	//m_sampler->bind();
	//m_shadowSampler->bind(ShaderComponent::PS, 1);

}

void DeferredDirectionalLightShader::setLight(const DirectionalLight& dl, Camera* camera) {

	LightDataBuffer data;
	// Transform the vector into view space using the cameras view matrix
	// Using TransformNormal since this is a direction, translations should not be applied
	data.direction = Vector3::TransformNormal(dl.getDirection(), camera->getViewMatrix());
	data.color = dl.getColor();
	
	setCBufferVar("def_dirLightInput", &data, sizeof(data));

}
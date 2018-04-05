#pragma once

#include <d3d11.h>
#include <SimpleMath.h>
#include "../ShaderSet.h"
#include "../component/ConstantBuffer.h"
#include "../component/Sampler.h"
#include "../../../api/Application.h"
//#include "../../Lights.h"
#include "../../geometry/Material.h"

class MaterialShader : public ShaderSet {
public:
	MaterialShader();
	~MaterialShader();

	void bind() override;

	virtual void setClippingPlane(const DirectX::SimpleMath::Vector4& clippingPlane);
	//void updateLights(const Lights& lights);

	// TODO: remove this
	/*struct Vertex {
		DirectX::SimpleMath::Vector3 position;
		DirectX::SimpleMath::Vector2 texCoords;
		DirectX::SimpleMath::Vector3 normal;
		DirectX::SimpleMath::Vector3 tangent;
		DirectX::SimpleMath::Vector3 bitangent;
	};*/

private:
	/*void updateModelDataBuffer(const Material& material, const DirectX::SimpleMath::Matrix& w, const DirectX::SimpleMath::Matrix& vp) const;
	void updateWorldDataBuffer() const;*/

private:
	// Transform constant buffer structure
	//struct ModelDataBuffer {
	//	DirectX::SimpleMath::Matrix mWorld;
	//	DirectX::SimpleMath::Matrix mVP;
	//	DirectX::SimpleMath::Vector4 modelColor;
	//	float ka;
	//	float kd;
	//	float ks;
	//	float shininess;
	//	int textureFlags[3];
	//	int padding;
	//};
	//struct WorldDataBuffer {
	//	DirectX::SimpleMath::Vector4 clippingPlane;
	//	DirectX::SimpleMath::Vector3 cameraPos;
	//	float padding;
	//};
	//struct DirLightBuffer {
	//	Lights::DirectionalLight dirLight;
	//};
	//struct PointLightsBuffer {
	//	Lights::PointLightStruct pLights[8]; // Max number of lights as set in shader
	//	float padding1, padding2, padding3, padding4;
	//};
	/*DirLightBuffer m_dirLightData;
	PointLightsBuffer m_pointLightsData;*/
	/*DirectX::SimpleMath::Matrix m_vpMatrix;*/
	DirectX::SimpleMath::Vector4 m_clippingPlane;
	//DirectX::SimpleMath::Vector3 m_cameraPos;
	bool m_clippingPlaneHasChanged;
	//bool m_cameraPosHasChanged;

	// Components

	/*std::unique_ptr<ShaderComponent::ConstantBuffer> m_modelDataBuffer;
	std::unique_ptr<ShaderComponent::ConstantBuffer> m_worldDataBuffer;
	std::unique_ptr<ShaderComponent::ConstantBuffer> m_lightsBuffer;*/
	//std::unique_ptr<ShaderComponent::Sampler> m_sampler;

};

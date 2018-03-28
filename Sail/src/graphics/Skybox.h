#pragma once

#include "../resources/DXCubeMap.h"
#include "shader/ShaderSet.h"
#include "camera/Camera.h"

class CubeMapShader;

class Skybox {
public:
	Skybox(const std::wstring& filename, CubeMapShader* cubeMapShader);
	~Skybox();

	void draw(Camera& cam);
	void updateCameraPos(const DirectX::SimpleMath::Vector3& camPos);

private:
	std::unique_ptr<Model> createSkyboxModel() const;

private:
	std::unique_ptr<DXCubeMap> m_cubeMap;
	std::unique_ptr<Model> m_model;
	CubeMapShader* m_cubeMapShader;

};
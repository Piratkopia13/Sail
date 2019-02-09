#include "Skybox.h"
#include "shader/basic/CubeMapShader.h"

using namespace DirectX;
using namespace SimpleMath;

Skybox::Skybox(const std::wstring& filename, CubeMapShader* cubeMapShader) {

	m_cubeMap = std::make_unique<DXCubeMap>(filename);
	m_cubeMapShader = cubeMapShader;
	m_model = createSkyboxModel();
	//m_model->buildBufferForShader(cubeMapShader);
	m_model->getMaterial()->setDiffuseTexture(*m_cubeMap->getResourceView());

}
Skybox::~Skybox() {
}

void Skybox::updateCameraPos(const DirectX::SimpleMath::Vector3& camPos) {
	m_model->getTransform().setTranslation(camPos);
}

void Skybox::draw(Camera& cam) {

	m_cubeMapShader->updateCamera(cam);
	updateCameraPos(cam.getPosition());

	Application::getInstance()->getDXManager()->disableDepthBuffer();
	m_model->draw(true);
	Application::getInstance()->getDXManager()->enableDepthBuffer();
}

std::unique_ptr<Model> Skybox::createSkyboxModel() const {
	const int numVerts = 8;

	Vector3 halfSizes(1.f, 1.f, 1.f);

	Vector3* positions = new Vector3[numVerts]{
		Vector3(-halfSizes.x, halfSizes.y, halfSizes.z),
		Vector3(-halfSizes.x, -halfSizes.y, halfSizes.z),
		Vector3(halfSizes.x, halfSizes.y, halfSizes.z),
		Vector3(halfSizes.x, -halfSizes.y, halfSizes.z),
		Vector3(-halfSizes.x, halfSizes.y, -halfSizes.z),
		Vector3(-halfSizes.x, -halfSizes.y, -halfSizes.z),
		Vector3(halfSizes.x, halfSizes.y, -halfSizes.z),
		Vector3(halfSizes.x, -halfSizes.y, -halfSizes.z)
	};

	const int numIndices = 36;
	ULONG* indices = new ULONG[numIndices]{
		0, 2, 1, 1, 2, 3,
		2, 6, 3, 3, 6, 7,
		6, 4, 7, 7, 4, 5,
		4, 0, 5, 5, 0, 1,
		1, 3, 5, 5, 3, 7,
		4, 6, 0, 0, 6, 2
	};

	Model::Data buildData;
	buildData.numVertices = numVerts;
	buildData.positions = positions;
	buildData.numIndices = numIndices;
	buildData.indices = indices;

	std::unique_ptr<Model> model = std::make_unique<Model>(buildData, m_cubeMapShader);

	return model;
}
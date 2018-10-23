#include "DeferredRenderer.h"
#include "../shader/ShaderSet.h"
#include "../light/LightSetup.h"
#include "../geometry/Model.h"
#include "../geometry/factory/ScreenQuadModel.h"
#include "../../api/Application.h"
#include "../shader/deferred/DeferredPointLightShader.h"
#include "../shader/deferred/DeferredDirectionalLightShader.h"
#include "../shader/deferred/DeferredGeometryShader.h"

using namespace DirectX::SimpleMath;

DeferredRenderer::DeferredRenderer() {

	auto& resman = Application::getInstance()->getResourceManager();
	m_pointLightShader = &resman.getShaderSet<DeferredPointLightShader>();
	m_dirLightShader = &resman.getShaderSet<DeferredDirectionalLightShader>();

	// Create light volume model
	m_pointLightVolumeModel = &resman.getModel("normalizedSphere.fbx", m_pointLightShader);

	// Create the fullscreen quad model used for light pass
	m_screenQuadModel = ModelFactory::ScreenQuadModel::Create(m_dirLightShader);

	auto* window = Application::getInstance()->getWindow();
	UINT width = window->getWindowWidth();
	UINT height = window->getWindowHeight();

	// Init renderable textures
	for (int i = 0; i < NUM_GBUFFERS - 1; i++) {
		m_gBuffers[i] = std::unique_ptr<RenderableTexture>(new RenderableTexture(1, width, height, (i == 0))); // Create a dsv for the diffuse gbuffer(0)
		m_gBufferRTVs[i] = *m_gBuffers[i]->getRenderTargetView(); // Store pointer to render target views
	}

}

DeferredRenderer::~DeferredRenderer() {

}

void DeferredRenderer::begin(Camera* camera) {
	m_camera = camera;
	commandQueue.clear();
}

void DeferredRenderer::submit(Mesh* mesh, const DirectX::SimpleMath::Matrix& modelMatrix) {
	RenderCommand cmd;
	cmd.mesh = mesh;
	cmd.transform = modelMatrix.Transpose();
	commandQueue.push_back(cmd);
	//Logger::Log("Submitted model at: " + Utils::vec3ToStr(commandQueue.back().modelMatrix.Transpose().Translation()));

}

void DeferredRenderer::submit(Model* model, const DirectX::SimpleMath::Matrix& modelMatrix) {
	Renderer::submit(model, modelMatrix);
}

void DeferredRenderer::setLightSetup(LightSetup* lightSetup) {
	m_lightSetup = lightSetup;
}

void DeferredRenderer::end() {
}

void DeferredRenderer::present() {

	beginGeometryPass();

	// Bind the deferred geometry shader and update camera variables
	DeferredGeometryShader* geometryShader = &Application::getInstance()->getResourceManager().getShaderSet<DeferredGeometryShader>();
	geometryShader->bind();
	geometryShader->setCBufferVar("sys_mView", &m_camera->getViewMatrix().Transpose(), sizeof(Matrix));
	geometryShader->setCBufferVar("sys_mProj", &m_camera->getProjMatrix().Transpose(), sizeof(Matrix));

	// loop and draw
	for (RenderCommand& command : commandQueue) {
		ShaderSet* shader = command.mesh->getMaterial()->getShader();
		// Make sure the mesh being rendered is set up to use the deferred geometry shader 
		assert((int)shader == (int)geometryShader);

		// Update world matrix
		geometryShader->setCBufferVar("sys_mWorld", &command.transform, sizeof(Matrix));

		command.mesh->draw(*this);
	}
	
	
	doLightPass();
	
}

void DeferredRenderer::beginGeometryPass() const {

	// The last RTV is the light pass RTV used to write ambient light without having an extra pass
	//m_rtvs[NUM_GBUFFERS - 1] = lightPassRTV;

	Application::getInstance()->getAPI()->getDeviceContext()->RSSetViewports(1, m_gBuffers[0]->getViewPort());

	auto dxm = Application::getInstance()->getAPI();
	dxm->setBlending(GraphicsAPI::NO_BLENDING);

	// Set render targets to all the gbuffers, except depth which is written to anyway
	dxm->getDeviceContext()->OMSetRenderTargets(NUM_GBUFFERS, m_gBufferRTVs, *m_gBuffers[DIFFUSE_GBUFFER]->getDepthStencilView());

	// Clear all gbuffers
	for (int i = 0; i < NUM_GBUFFERS - 1; i++)
		m_gBuffers[i]->clear({ 0.1f, 0.2f, 0.3f, 1.0f });

	// Update the camera in the shaders
	//shader->setCBufferVar("sys_mWorld", &command.transform, sizeof(Matrix));
	m_pointLightShader->bind();
	m_pointLightShader->setCBufferVar("sys_mView", &m_camera->getViewMatrix().Transpose(), sizeof(Matrix));
	m_pointLightShader->setCBufferVar("sys_mProj", &m_camera->getProjMatrix().Transpose(), sizeof(Matrix));

	m_dirLightShader->bind();
	m_dirLightShader->setCBufferVar("sys_mInvProj", &m_camera->getProjMatrix().Transpose().Invert(), sizeof(Matrix));

	// Bind the geometry shader
	//Application::getInstance()->getResourceManager().getShaderSet<DeferredGeometryShader>().bind();
}

void DeferredRenderer::doLightPass() {

	auto* dxm = Application::getInstance()->getAPI();
	auto* devCon = dxm->getDeviceContext();

	dxm->renderToBackBuffer();

	/*if (dlShadowMap)
		devCon->PSSetShaderResources(10, 1, dlShadowMap->getSRV());*/
	//m_dirLightShader.updateCameraBuffer(cam, lights.getDirectionalLightCamera());

	dxm->setDepthMask(GraphicsAPI::BUFFER_DISABLED);

	// Bind for dir light rendering pass
	m_dirLightShader->bind();
	// Do the directional light
	m_dirLightShader->setLight(m_lightSetup->getDL(), m_camera);

	m_dirLightShader->setTexture2D("def_texDiffuse", *m_gBuffers[DIFFUSE_GBUFFER]->getColorSRV());
	m_dirLightShader->setTexture2D("def_texNormal", *m_gBuffers[NORMAL_GBUFFER]->getColorSRV());
	m_dirLightShader->setTexture2D("def_texSpecular", *m_gBuffers[SPECULAR_GBUFFER]->getColorSRV());
	m_dirLightShader->setTexture2D("def_texDepth", *m_gBuffers[DIFFUSE_GBUFFER]->getDepthSRV());

	// Draw all pixels on the screen since the directional light affects them all
	m_screenQuadModel->draw(*this);

	dxm->setBlending(GraphicsAPI::ADDITIVE);

	// Bind for point light rendering pass
	m_pointLightShader->bind();

	m_pointLightShader->setTexture2D("def_texDiffuse", *m_gBuffers[DIFFUSE_GBUFFER]->getColorSRV());
	m_pointLightShader->setTexture2D("def_texNormal", *m_gBuffers[NORMAL_GBUFFER]->getColorSRV());
	m_pointLightShader->setTexture2D("def_texSpecular", *m_gBuffers[SPECULAR_GBUFFER]->getColorSRV());
	m_pointLightShader->setTexture2D("def_texDepth", *m_gBuffers[DIFFUSE_GBUFFER]->getDepthSRV());

	for (const PointLight& pl : m_lightSetup->getPLs()) {

		m_pointLightShader->setLight(pl, m_camera);

		Matrix transform;
		transform *= Matrix::CreateScale(pl.getRadius());
		transform *= Matrix::CreateTranslation(pl.getPosition());

		m_pointLightShader->setCBufferVar("sys_mWorld", &transform.Transpose(), sizeof(Matrix));
		m_pointLightVolumeModel->draw(*this);

	}

	dxm->setDepthMask(GraphicsAPI::NO_MASK);
	dxm->setBlending(GraphicsAPI::NO_BLENDING);

	ID3D11ShaderResourceView* nullSRVs[4] = { nullptr, nullptr, nullptr, nullptr };
	devCon->PSSetShaderResources(0, 4, nullSRVs);

}

void DeferredRenderer::onEvent(Event& event) {
	EventHandler::dispatch<WindowResizeEvent>(event, FUNC(&DeferredRenderer::onResize));
}

bool DeferredRenderer::onResize(WindowResizeEvent& event) {

	unsigned int width = event.getWidth();
	unsigned int height = event.getHeight();

	// Update texture sizes
	for (int i = 0; i < NUM_GBUFFERS - 1; i++) {
		m_gBuffers[i]->resize(width, height);
		m_gBufferRTVs[i] = *m_gBuffers[i]->getRenderTargetView(); // Store new rtv pointer
	}

	return false;
}


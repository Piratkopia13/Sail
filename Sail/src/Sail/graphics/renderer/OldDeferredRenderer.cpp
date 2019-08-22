#include "DeferredRenderer.h"
#include "../../Application.h"

using namespace DirectX;
using namespace SimpleMath;

DeferredRenderer::DeferredRenderer(){

	// Create light volumes
	// TODO: fix
	/*m_pointLightVolume = std::make_unique<FbxModel>("normalizedSphere.fbx");
	m_pointLightVolume->getModel()->buildBufferForShader(&Application::getInstance()->getResourceManager().getShaderSet<DeferredPointLightShader>());*/

	// Create full screen quad model for directional light
	createFullscreenQuad();

	auto window = Application::getInstance()->getWindow();
	UINT width = window->getWindowWidth();
	UINT height = window->getWindowHeight();

	// Init renderable textures
	m_gBuffers[0] = std::unique_ptr<RenderableTexture>(new RenderableTexture(1, width, height, true));
	for (int i = 1; i < NUM_GBUFFERS - 1; i++) {
		m_gBuffers[i] = std::unique_ptr<RenderableTexture>(new RenderableTexture(1, width, height, false));
	}

	// Set up render target view array, shader resource view array and the depth stencil view
	for (int i = 0; i < NUM_GBUFFERS - 1; i++) {
		m_rtvs[i] = *m_gBuffers[i]->getRenderTargetView();
		m_srvs[i] = *m_gBuffers[i]->getColorSRV();
	}
	// Last SRV is the depth buffer
	m_srvs[DEPTH_GBUFFER] = *m_gBuffers[0]->getDepthSRV();
	m_dsv = *m_gBuffers[0]->getDepthStencilView();
	m_dsvSrv = *m_gBuffers[0]->getDepthSRV();
	
	//m_fullScreenPlane->getMaterial()->setTextures(m_srvs, NUM_GBUFFERS); // TODO: FIX
	//m_pointLightVolume->getModel()->getMaterial()->setTextures(m_srvs, NUM_GBUFFERS); // TODO: FIX

	/*ID3D11Texture2D* depthTextures[2];
	depthTextures[0] = m_gBuffers[0]->getDepthTexture2D();
	depthTextures[1] = ;

	m_dirLightShader.createTextureArray(width, height, depthTextures);*/

}
DeferredRenderer::~DeferredRenderer() {

}

void DeferredRenderer::beginGeometryPass(Camera& camera, ID3D11RenderTargetView* const lightPassRTV, ID3D11DepthStencilView* const dsv) {

	// The last RTV is the light pass RTV used to write ambient light without having an extra pass
	m_rtvs[NUM_GBUFFERS - 1] = lightPassRTV;

	Application::getInstance()->getDXManager()->getDeviceContext()->RSSetViewports(1, m_gBuffers[0]->getViewPort());

	auto dxm = Application::getInstance()->getDXManager();
	dxm->disableAlphaBlending();

	// Set render targets to all the gbuffers, except depth which is written to anyway
	dxm->getDeviceContext()->OMSetRenderTargets(NUM_GBUFFERS, m_rtvs, (dsv) ? dsv : m_dsv );

	// Clear all gbuffers
	for (int i = 0; i < NUM_GBUFFERS - 1; i++)
		m_gBuffers[i]->clear({0.f, 0.f, 0.0f, 0.0f});

	// Update the camera in the shaders
	Application::getInstance()->getResourceManager().getShaderSet<DeferredGeometryShader>().updateCamera(camera);
	Application::getInstance()->getResourceManager().getShaderSet<DeferredDirectionalLightShader>().updateCamera(camera);
	Application::getInstance()->getResourceManager().getShaderSet<DeferredPointLightShader>().updateCamera(camera);

	// Bind the geometry shader
	Application::getInstance()->getResourceManager().getShaderSet<DeferredGeometryShader>().bind();

}

void DeferredRenderer::beginLightDepthPass(ID3D11DepthStencilView* const dsv) {
	auto dxm = Application::getInstance()->getDXManager();

	dxm->getDeviceContext()->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH, 1.0f, 0);
	dxm->getDeviceContext()->OMSetRenderTargets(0, nullptr, dsv);

}

void DeferredRenderer::doLightPass(LightSetup& lights, Camera& cam, DirLightShadowMap* dlShadowMap) {

	auto devCon = Application::getInstance()->getDXManager()->getDeviceContext();

	DeferredDirectionalLightShader& dirLightShader = Application::getInstance()->getResourceManager().getShaderSet<DeferredDirectionalLightShader>();
	DeferredPointLightShader& pointLightShader = Application::getInstance()->getResourceManager().getShaderSet<DeferredPointLightShader>();

	if (dlShadowMap)
		devCon->PSSetShaderResources(10, 1, dlShadowMap->getSRV());
	dirLightShader.updateCameraBuffer(cam, lights.getDirectionalLightCamera());

	auto* dxm = Application::getInstance()->getDXManager();	
	dxm->disableDepthBuffer();
	dxm->enableAdditiveBlending();

	// Bind for dir light rendering pass
	dirLightShader.bind();
	// Do the directional light
	dirLightShader.setLight(lights.getDL());

	// Draw all pixels on the screen since the directional light affects them all
	//dirLightShader.draw(*m_fullScreenPlane, false); // TODO: FIX
// 	m_fullScreenPlane.draw(false);

	// Bind for point light rendering pass
	pointLightShader.bind();

	for (const LightSetup::PointLight& pl : lights.getPLs()) {
		pointLightShader.setLight(pl);

		// TODO: FIX
		/*m_pointLightVolume->getModel()->getTransform().setTranslation(pl.getPosition());
		m_pointLightVolume->getModel()->getTransform().setScale(pl.getRadius());
		m_pointLightVolume->getModel()->draw(false);*/

	}

	dxm->enableDepthBuffer();
	dxm->disableAlphaBlending();

	ID3D11ShaderResourceView* nullSRV = nullptr;
	devCon->PSSetShaderResources(10, 1, &nullSRV);
}

void DeferredRenderer::resize(int width, int height) {

	// Update texture sizes
	for (int i = 0; i < NUM_GBUFFERS - 1; i++) {
		m_gBuffers[i]->resize(width, height);
	}

	// Update render target view array, shader resource view array and the depth stencil view
	for (int i = 0; i < NUM_GBUFFERS - 1; i++) {
		m_rtvs[i] = *m_gBuffers[i]->getRenderTargetView();
		m_srvs[i] = *m_gBuffers[i]->getColorSRV();
	}
	// Last SRV is the depth buffer
	m_srvs[DEPTH_GBUFFER] = *m_gBuffers[0]->getDepthSRV();
	m_dsv = *m_gBuffers[0]->getDepthStencilView();
	m_dsvSrv = *m_gBuffers[0]->getDepthSRV();

	// Update the texture array in the directional light shader
	/*ID3D11Texture2D* gbufferTextures[NUM_GBUFFERS];
	for (int i = 0; i < NUM_GBUFFERS - 1; i++) {
		gbufferTextures[i] = m_gBuffers[i]->getTexture2D();
	}
	dirLightShader.createTextureArray(width, height, gbufferTextures);*/

}

//DeferredGeometryShader& DeferredRenderer::getGeometryShader() {
//	return m_geometryShader;
//}
//DeferredDirectionalLightShader& DeferredRenderer::getDirLightShader() {
//	return m_dirLightShader;
//}
//DeferredPointLightShader& DeferredRenderer::getPointLightShader() {
//	return m_pointLightShader;
//}

ID3D11ShaderResourceView** DeferredRenderer::getGBufferSRV(UINT index) {
	if (index > NUM_GBUFFERS) return nullptr;
	return &m_srvs[index];
}

RenderableTexture* DeferredRenderer::getGBufferRenderableTexture(UINT index)
{
	if (index > NUM_GBUFFERS) return nullptr;
	return m_gBuffers[index].get();
}

void DeferredRenderer::createFullscreenQuad() {

	Vector2 halfSizes(1.f, 1.f);

	const int numVerts = 4;
	Vector3* positions = new Vector3[numVerts]{
		Vector3(-halfSizes.x, -halfSizes.y, 0.f),
		Vector3(-halfSizes.x, halfSizes.y, 0.f),
		Vector3(halfSizes.x, -halfSizes.y, 0.f),
		Vector3(halfSizes.x, halfSizes.y, 0.f),
	};

	const int numIndices = 6;
	ULONG* indices = new ULONG[numIndices]{
		0, 1, 2, 2, 1, 3
	};

	Vector2* texCoords = new Vector2[numVerts]{
		Vector2(0.f, 1.f),
		Vector2(0.f, 0.f),
		Vector2(1.f, 1.f),
		Vector2(1.f, 0.f)
	};

	Model::Data data;
	data.numVertices = numVerts;
	data.numIndices = numIndices;
	data.positions = positions;
	data.indices = indices;
	data.texCoords = texCoords;

	//m_fullScreenPlane = std::make_unique<Model>(data, &Application::getInstance()->getResourceManager().getShaderSet<DeferredPointLightShader>());
	//m_fullScreenPlane.buildBufferForShader(&Application::getInstance()->getResourceManager().getShaderSet<DeferredPointLightShader>());
	
}

ID3D11DepthStencilView* const DeferredRenderer::getDSV() const {
	return m_dsv;
}
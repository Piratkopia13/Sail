#pragma once
#include "Scene.h"
#include "objects/common/Object.h"
#include "ParticleHandler.h"
#include "objects/Block.h"

using namespace std;

Scene::Scene(const AABB& worldSize)
	: m_dirLightShadowMap(10, 10)
	, m_doPostProcessing(true)
	, m_doShadows(false)
{

	m_spriteBatch = std::make_unique<DirectX::SpriteBatch>(Application::getInstance()->getDXManager()->getDeviceContext());
	m_timer.startTimer();

	// Camera rotation
	m_rotation = 0.f;

	auto window = Application::getInstance()->getWindow();
	UINT width = window->getWindowWidth();
	UINT height = window->getWindowHeight();

	m_deferredOutputTex = std::unique_ptr<RenderableTexture>(new RenderableTexture(1U, width, height, false));
	m_particleOutputTex = std::unique_ptr<RenderableTexture>(new RenderableTexture(1U, width, height, false));
}
Scene::~Scene() {}

void Scene::addObject(Object* newObject) {
	m_objects.push_back(newObject);
}

void Scene::addText(Text* text) {
	m_texts.push_back(text);
}

void Scene::addSkybox(const std::wstring& filename) {
	//m_cubeMapShader = make_unique<CubeMapShader>(); // TODO: uncomment
	//m_skybox = make_unique<Skybox>(filename, m_cubeMapShader.get()); // TODO: uncomment
}

void Scene::resize(int width, int height) {
	// Resize textures
	m_deferredRenderer.resize(width, height);
	m_deferredOutputTex->resize(width, height);
	m_particleOutputTex->resize(width, height);
	//m_postProcessPass.resize(width, height); // TODO: uncomment
}

// Draws the scene
// void Scene::draw(float dt, Camera& cam, Level* level, ProjectileHandler* projectiles, Gamemode* gamemode, ParticleHandler* particles) {
// 
// 	auto* dxm = Application::getInstance()->getDXManager();
// 
// 	dxm->getPerfProfilerThing()->BeginEvent(L"Scene rendering starting");
// 
// 	//dxm->getDeviceContext()->ClearState();
// 
// 	if (m_doPostProcessing) {
// 		m_postProcessPass.setCamera(cam);
// 		// Render skybox to the prePostTex
// 		m_deferredOutputTex->clear({ 0.0f, 0.0f, 0.0f, 1.0f });
// 		m_particleOutputTex->clear({ 0.f, 0.f, 0.f, 1.0f });
// 		dxm->getDeviceContext()->OMSetRenderTargets(1, m_deferredOutputTex->getRenderTargetView(), dxm->getDepthStencilView());
// 	}
// 
// 	// Update and render skybox if one is set
// 	// The skybox needs to be rendered first in the scene since it should be behind all models
// 	if (m_skybox)
// 		m_skybox->draw(cam);
// 		
// 
// 	if (m_doShadows) {
// 		// Renders the depth of the scene out of the directional lights position
// 
// 		//To-do: Fix shadow pass to work with draw call from object
// 		/*m_deferredRenderer.beginLightDepthPass(*m_dirLightShadowMap.getDSV());
// 		dxm->getDeviceContext()->RSSetViewports(1, m_dirLightShadowMap.getViewPort());
// 		m_depthShader.bind();
// 		dxm->enableFrontFaceCulling();*/
// 
// 		// Render all blocks to the shadow map
// 		// TODO: only render the blocks that the camera can see
// 		/*if (level) {
// 			auto& blocks = level->getGrid()->getAllBlocks();
// 			for (auto& row : blocks) {
// 				for (auto* block : row) {
// 					if (block) {
// 						block->getModel()->setTransform(&block->getTransform());
// 						m_depthShader.draw(*block->getModel(), false);
// 					}
// 				}
// 			}
// 		}*/
// 		dxm->enableBackFaceCulling();
// 	}
// 
// 	// Begin geometry pass - store depth in the correct texture
// 	if (m_doPostProcessing) {
// 		m_deferredRenderer.beginGeometryPass(cam, *m_deferredOutputTex->getRenderTargetView());
// 	}
// 	else {
// 		m_deferredRenderer.beginGeometryPass(cam, *dxm->getBackBufferRTV());
// 	}
// 
// 	m_timer.getFrameTime();
// 	/* draw level here */
// 	dxm->getPerfProfilerThing()->BeginEvent(L"Level rendering");
// 	if (level) {
// 		level->draw();
// 	}
// 	// Disable conservatiec rasterization to avoid wierd graphical artifacts
// 	dxm->disableConservativeRasterizer();
// 	dxm->getPerfProfilerThing()->EndEvent();
// 	dxm->getPerfProfilerThing()->BeginEvent(L"Gamemode rendering");
// 	if (gamemode) {
// 		gamemode->draw();
// 	}
// 	dxm->getPerfProfilerThing()->EndEvent();
// 	dxm->getPerfProfilerThing()->BeginEvent(L"Projectile rendering");
// 	if (projectiles) {
// 		projectiles->draw();
// 	}
// 	dxm->getPerfProfilerThing()->EndEvent();
// 	dxm->getPerfProfilerThing()->BeginEvent(L"Other object rendering");
// 	for (Object* m : m_objects)
// 		m->draw();
// 	dxm->getPerfProfilerThing()->EndEvent();
// 
// 	dxm->enableAlphaBlending();
// 
// 	// Render particles to separate texture if post processing is active
// 	// This is neccassary for the depth of field effect since it needs depth data that particles dont write (will always be in focus)
// 	if (m_doPostProcessing)
// 		dxm->getDeviceContext()->OMSetRenderTargets(1, m_particleOutputTex->getRenderTargetView(), m_deferredRenderer.getDSV());
// 	dxm->getPerfProfilerThing()->BeginEvent(L"Particle rendering");
// 	if (particles) {
// 		particles->draw();
// 	}
// 	dxm->getPerfProfilerThing()->EndEvent();
// 	//double time = m_timer.getFrameTime();
// 	//std::cout << "Rendering took: " << time * 1000.f << "ms" << std::endl << std::endl;
// 
// 	// Switch render target to where the deferred output should be
// 	if (m_doPostProcessing) {
// 		//m_prePostTex->clear({ 0.f, 0.f, 0.f, 0.0f });
// 		dxm->getDeviceContext()->OMSetRenderTargets(1, m_deferredOutputTex->getRenderTargetView(), dxm->getDepthStencilView());
// 	}
// 	else {
// 		dxm->renderToBackBuffer();
// 	}
// 
// 	// Do the light pass (using additive blending)
// 	dxm->getPerfProfilerThing()->BeginEvent(L"Deferred light pass");
// 	m_deferredRenderer.doLightPass(m_lights, cam, (m_doShadows) ? &m_dirLightShadowMap : nullptr);
// 	dxm->getPerfProfilerThing()->EndEvent();
// 
// 	if (m_doPostProcessing) {
// 		// Do post processing
// 		//m_deferredOutputTex->clear({ 0.f, 0.f, 0.f, 0.0f });
// 
// 		// Unbind deferredOutputTex so that it can be bound to UAVs in the post process pass
// 		ID3D11RenderTargetView* nullRTV = nullptr;
// 		dxm->getDeviceContext()->OMSetRenderTargets(1, &nullRTV, nullptr);
// 		dxm->getPerfProfilerThing()->BeginEvent(L"Post process pass");
// 		// Run post process pass
// 		m_postProcessPass.run(*m_deferredOutputTex, m_deferredRenderer.getGBufferSRV(DeferredRenderer::DEPTH_GBUFFER), *m_deferredRenderer.getGBufferRenderableTexture(DeferredRenderer::DIFFUSE_GBUFFER), *m_particleOutputTex);
// 		dxm->getPerfProfilerThing()->EndEvent();
// 	}
// 
// 	// Re-enable conservative rasterization
// 	dxm->enableBackFaceCulling();
// 
// 	// Change active depth buffer to the one used in the deferred geometry pass
// 	dxm->getDeviceContext()->OMSetRenderTargets(1, dxm->getBackBufferRTV(), m_deferredRenderer.getDSV());
// 
// 	dxm->getPerfProfilerThing()->EndEvent();
// }

void Scene::drawHUD() {
	auto* dxm = Application::getInstance()->getDXManager();

	dxm->getPerfProfilerThing()->BeginEvent(L"HUD rendering starting");

	// 2D rendering stuffs
	// Beginning the spritebatch will disable depth testing
	if (!m_texts.empty()) {
		m_spriteBatch->Begin();
		for (Text* text : m_texts)
			text->draw(m_spriteBatch.get());
		m_spriteBatch->End();
	}

	// Re-enable the depth buffer and rasterizer state after 2D rendering
	dxm->enableDepthBuffer();
	dxm->enableBackFaceCulling();

	dxm->getPerfProfilerThing()->EndEvent();
}

std::map<ShaderSet*, std::vector<Model*>> Scene::mapModelsToShaders(std::vector<Quadtree::Element*>& elements) {

	std::map<ShaderSet*, std::vector<Model*>> mappedModels;

	for (Quadtree::Element* e : elements) {

		ShaderSet* shader = e->model->getShader();

		auto it = mappedModels.find(shader);
		if (it == mappedModels.end())
			mappedModels.insert({ shader,{ e->model } });
		else
			it->second.push_back(e->model);

	}

	return mappedModels;

}

void Scene::setUpDirectionalLight(const Lights::DirectionalLight& dl) {

	m_lights.setDirectionalLight(dl);

	// Set up shadow map camera
	float w = 100.f;
	OrthographicCamera dlCam(w, w / 1.9f, -105.f, 110.f);
	dlCam.setPosition(-m_lights.getDL().direction * 5.f);
	dlCam.setDirection(m_lights.getDL().direction);
	m_lights.setDirectionalLightCamera(dlCam);

	//m_depthShader.updateCamera(dlCam); // TODO: uncomment
}

Lights& Scene::getLights() {
	return m_lights;
}

DeferredRenderer& Scene::getDeferredRenderer() {
	return m_deferredRenderer;
}

DirLightShadowMap& Scene::getDLShadowMap() {
	return m_dirLightShadowMap;
}

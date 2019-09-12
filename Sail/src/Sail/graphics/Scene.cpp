#include "pch.h"
#include "Scene.h"
#include "../entities/components/Components.h"
#include "geometry/Model.h"
#include "light/LightSetup.h"
#include "../utils/Utils.h"
#include "Sail/Application.h"
#include "Sail/api/Renderer.h"


Scene::Scene() 
	//: m_postProcessPipeline(m_renderer)
{
	m_renderer = std::unique_ptr<Renderer>(Renderer::Create(Renderer::FORWARD));

	// TODO: the following method ish
	//m_postProcessPipeline.add<FXAAStage>();
	/*m_postProcessPipeline.add<GaussianBlurStage>(1.f / 1.f);
	m_postProcessPipeline.add<GaussianBlurStage>(1.f / 1.5f);
	m_postProcessPipeline.add<GaussianBlurStage>(1.f / 2.f);*/
	//m_postProcessPipeline.add<FXAAStage>();

	auto window = Application::getInstance()->getWindow();
	UINT width = window->getWindowWidth();
	UINT height = window->getWindowHeight();

	//m_deferredOutputTex = std::unique_ptr<DX11RenderableTexture>(SAIL_NEW DX11RenderableTexture(1U, width, height, false));

}

Scene::~Scene() {

}

void Scene::addEntity(Entity::SPtr entity) {
	m_entities.push_back(entity);
}

void Scene::setLightSetup(LightSetup* lights) {
	m_renderer->setLightSetup(lights);
}

// TODO: Move matrix updates to its own system and optimize it more, only update matrices that need to be updated
// TODO: use the alpha value to interpolate between game states
void Scene::draw(Camera& camera, const float alpha) {
	const UINT nextInd = Application::getInstance()->getSnapshotBufferIndex();
	const UINT currentInd = (nextInd + SNAPSHOT_BUFFER_SIZE - 1) % SNAPSHOT_BUFFER_SIZE;
	const UINT prevInd = (nextInd + SNAPSHOT_BUFFER_SIZE - 2) % SNAPSHOT_BUFFER_SIZE;

	m_renderer->begin(&camera);

	// TODO: Should be in a prepare render stage:
	for (Entity::SPtr& entity : m_entities) {
		ModelComponent* model = entity->getComponent<ModelComponent>();
		if (model) {
			// Update all entities that have transform matrices
			TransformDataComponent* data = entity->getComponent<TransformDataComponent>();
			TransformMatrixComponent* matrix = entity->getComponent<TransformMatrixComponent>();
			if (data && matrix) {
				m_renderer->submit(model->getModel(), data->getMatrixFromData(currentInd, alpha));
			}
		}
	}


	m_renderer->end();
	m_renderer->present();
	//m_renderer->present(m_deferredOutputTex.get());

	//m_postProcessPipeline.run(*m_deferredOutputTex, nullptr);

	// Draw text last
	// TODO: sort entity list instead of iterating entire list twice
	for (Entity::SPtr& entity : m_entities) {
		TextComponent* text = entity->getComponent<TextComponent>();
		if (text) {
			text->draw();
		}
	}
}

bool Scene::onEvent(Event& event) {
	EventHandler::dispatch<WindowResizeEvent>(event, SAIL_BIND_EVENT(&Scene::onResize));

	// Forward events
	m_renderer->onEvent(event);
	//m_postProcessPipeline.onEvent(event);

	return true;
}

bool Scene::onResize(WindowResizeEvent & event) {

	unsigned int width = event.getWidth();
	unsigned int height = event.getHeight();

	//m_deferredOutputTex->resize(width, height);

	return false;
}

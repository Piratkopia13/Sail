#include "pch.h"
#include "Scene.h"
#include "../entities/components/Components.h"
#include "geometry/Model.h"
#include "light/LightSetup.h"
#include "../utils/Utils.h"
#include "Sail/Application.h"
#include "Sail/api/Renderer.h"
#include "Sail/entities/ECS.h"


Scene::Scene()
	: m_doPostProcessing(false)
{
	m_rendererRaster = std::unique_ptr<Renderer>(Renderer::Create(Renderer::FORWARD));
	m_rendererRaytrace = std::unique_ptr<Renderer>(Renderer::Create(Renderer::RAYTRACED));
	m_currentRenderer = &m_rendererRaster;

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

	m_showBoundingBoxes = false;
}

Scene::~Scene() {

}

void Scene::addEntity(Entity::SPtr entity) {
	m_sceneEntities.push_back(entity);
}

void Scene::setLightSetup(LightSetup* lights) {
	m_rendererRaster->setLightSetup(lights);
	m_rendererRaytrace->setLightSetup(lights);
}

// NEEDS TO RUN ONCE BEFORE EACH UPDATE-LOOP ITERATION
// Copies the game state from the previous tick 
//void Scene::prepareUpdate() {
//	for (auto e : m_sceneEntities) {
//		TransformComponent* transform = e->getComponent<TransformComponent>();
//		if (transform) { transform->prepareUpdate(); }
//	}
//}

void Scene::showBoundingBoxes(bool val) {
	m_showBoundingBoxes = val;
}

// alpha is a the interpolation value (range [0,1]) between the last two snapshots
void Scene::draw(Camera& camera, const float alpha) {
	(*m_currentRenderer)->begin(&camera);

	for (Entity::SPtr entity : m_sceneEntities) {
		ModelComponent* model = entity->getComponent<ModelComponent>();
		TransformComponent* transform = entity->getComponent<TransformComponent>();

		if (model && transform) {
			if (entity->getComponent<RealTimeComponent>()) {
				// If it's a real-time entity render with the most recent update
				// Not that for these entities should be updated once per frame for this to work correctly
				(*m_currentRenderer)->submit(model->getModel(), transform->getMatrix());
			} else {
				// If not interpolate between the two most recent updates
				(*m_currentRenderer)->submit(model->getModel(), transform->getRenderMatrix(alpha));
			}
		}

		if (m_showBoundingBoxes) {
			BoundingBoxComponent* boundingBox = entity->getComponent<BoundingBoxComponent>();
			if (boundingBox) {
				Model* wireframeModel = boundingBox->getWireframeModel();
				if (wireframeModel) {
					// Bounding boxes are visualized with their most update since that's what's used for hit detection
					(*m_currentRenderer)->submit(wireframeModel, boundingBox->getTransform()->getMatrix());
				}
			}
		}
	}

	(*m_currentRenderer)->end();
	(*m_currentRenderer)->present((m_doPostProcessing) ? &m_postProcessPipeline : nullptr);

	// Draw text last
	// TODO: sort entity list instead of iterating entire list twice
	/*for (Entity::SPtr& entity : m_entities) {
		TextComponent* text = entity->getComponent<TextComponent>();
		if (text) {
			text->draw();
		}
	}*/
}

//TODO add failsafe
Entity::SPtr Scene::getSceneEntityByName(std::string name) {
	for (auto e : m_sceneEntities) {
		if (e->getName() == name) {
			return e;
		}
	}
	return NULL;
}

const std::vector<Entity::SPtr>& Scene::getGameObjectEntities() const {
	return m_sceneEntities;
}

void Scene::draw(void) {
	(*m_currentRenderer)->begin(nullptr);
	(*m_currentRenderer)->end();
	(*m_currentRenderer)->present();
}

bool Scene::onEvent(Event& event) {
	EventHandler::dispatch<WindowResizeEvent>(event, SAIL_BIND_EVENT(&Scene::onResize));

	// Forward events
	m_rendererRaster->onEvent(event);
	m_rendererRaytrace->onEvent(event);
	//m_postProcessPipeline.onEvent(event);

	return true;
}

void Scene::changeRenderer(unsigned int index) {
	if (index == 0) {
		m_currentRenderer = &m_rendererRaster;
	}
	else {
		m_currentRenderer = &m_rendererRaytrace;
	}
}

bool& Scene::getDoProcessing() {
	return m_doPostProcessing;
}

bool Scene::onResize(WindowResizeEvent& event) {

	unsigned int width = event.getWidth();
	unsigned int height = event.getHeight();

	//m_deferredOutputTex->resize(width, height);

	return false;
}

#include "pch.h"
#include "Scene.h"
#include "../entities/components/Components.h"
#include "geometry/Model.h"
#include "light/LightSetup.h"
#include "../utils/Utils.h"
#include "Sail/Application.h"
#include "Sail/api/Renderer.h"
#include "Sail/entities/ECS.h"
#include "Sail/graphics/geometry/PerUpdateRenderObject.h"


// STATIC FUNCTIONS
std::atomic_uint Scene::s_frameIndex = 0;
UINT Scene::s_updateIndex = 0;
UINT Scene::s_renderIndex = 0;

// To be done at the end of each CPU update and nowhere else	
void Scene::IncrementCurrentUpdateIndex() {
	s_frameIndex++;
	s_updateIndex = s_frameIndex.load() % SNAPSHOT_BUFFER_SIZE;
}

// To be done just before render is called
void Scene::UpdateCurrentRenderIndex() {
	s_renderIndex = prevInd(s_frameIndex.load());
}

//#ifdef _DEBUG
UINT Scene::GetUpdateIndex() { return s_updateIndex; }
UINT Scene::GetRenderIndex() { return s_renderIndex; }
//#endif


// NON-STATIC FUNCTIONS

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
	m_gameObjectEntities.push_back(entity);
}

void Scene::addStaticEntity(Entity::SPtr staticEntity) {
	m_staticObjectEntities.push_back(staticEntity);
}

void Scene::setLightSetup(LightSetup* lights) {
	m_renderer->setLightSetup(lights);
}


// NEEDS TO RUN BEFORE EACH UPDATE
// Copies the game state from the previous tick 
void Scene::prepareUpdate() {
	for (auto e : m_gameObjectEntities) {
		TransformComponent* transform = e->getComponent<TransformComponent>();
		if (transform) { transform->prepareUpdate(); }
	}
}

// creates a vector of render objects corresponding to the current state of the game objects.
// Should be done at the end of each update tick.
void Scene::prepareRenderObjects() {
	const UINT ind = Scene::GetUpdateIndex();
	m_perFrameLocks[ind].lock();
	m_dynamicRenderObjects[ind].clear();

	size_t test = m_dynamicRenderObjects[ind].size();

	// Push dynamic objects' transform snapshots and model pointers to a transient vector
	for (auto gameObject : m_gameObjectEntities) {
		TransformComponent* transform = gameObject->getComponent<TransformComponent>();
		ModelComponent* model = gameObject->getComponent<ModelComponent>();
		if (transform && model) {
			m_dynamicRenderObjects[ind].push_back(PerUpdateRenderObject(transform, model));
		}
	}
	m_perFrameLocks[ind].unlock();

}

// alpha is a the interpolation value (range [0,1]) between the last two snapshots
void Scene::draw(Camera& camera, const float alpha) {
	m_renderer->begin(&camera);

	// Render static objects (essentially the map)
	// Matrices aren't changed between frames
	for (Entity::SPtr entity : m_staticObjectEntities) {
		ModelComponent* model = entity->getComponent<ModelComponent>();
		StaticMatrixComponent* matrix = entity->getComponent<StaticMatrixComponent>();

		if (model && matrix) {
			m_renderer->submit(model->getModel(), matrix->getMatrix());
		}
	}

	// Render dynamic objects (objects that might move or be added/removed)
	// Matrices are created from interpolated data each frame
	const UINT ind = Scene::GetRenderIndex();
	m_perFrameLocks[ind].lock();
	for (PerUpdateRenderObject& obj : m_dynamicRenderObjects[ind]) {
		m_renderer->submit(obj.getModel(), obj.getMatrix(alpha));
	}
	m_perFrameLocks[ind].unlock();


	m_renderer->end();
	m_renderer->present();

	//m_postProcessPipeline.run(*m_deferredOutputTex, nullptr);

	// Draw text last
	// TODO: sort entity list instead of iterating entire list twice
	for (Entity::SPtr& entity : m_gameObjectEntities) {
		TextComponent* text = entity->getComponent<TextComponent>();
		if (text) {
			text->draw();
		}
	}
}

//TODO add failsafe
Entity::SPtr Scene::getGameObjectEntityByName(std::string name) {
	for (auto e : m_gameObjectEntities) {
		if (e->getName() == name) {
			return e;
		}
	}
	return NULL;
}

const std::vector<Entity::SPtr>& Scene::getGameObjectEntities() const {
	return m_gameObjectEntities;
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

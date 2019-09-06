#pragma once

#include <vector>
#include <memory>
#include <map>
#include "Sail.h"

class Gamemode;
class ProjectileHandler;
class ParticleHandler;
class Object;
class Level;
class Scene {

public:
	Scene(const AABB& worldSize);
	~Scene();

	// Adds the specified model to the scene
	void addObject(Object* newObject);

	// Adds the specified text to the scene
	// This does not take ownership of the object
	void addText(Text* text);

	// Adds a skybox using the specified cube texture
	void addSkybox(const std::wstring& filename);

	// Draws the scene
// 	void draw(float dt, Camera& cam, Level* level = nullptr, ProjectileHandler* projectiles = nullptr, Gamemode* gamemode = nullptr, ParticleHandler* particles = nullptr);
	// Draws the HUD
	void drawHUD();

	// Resizes textures used
	// This has to be called on window  
	void resize(int width, int height);

	// Return the lights
	Lights& getLights();
	// Return the deferred renderer
	DeferredRenderer& getDeferredRenderer();

	DirLightShadowMap& getDLShadowMap();

	// Setup the DL for shadows
	void setUpDirectionalLight(const Lights::DirectionalLight& dl);

private:

	std::map<ShaderSet*, std::vector<Model*>> mapModelsToShaders(std::vector<Quadtree::Element*>& elements);

private:
	DeferredRenderer m_deferredRenderer;
	Timer m_timer;

	//std::map<ShaderSet*, std::vector<Model*>> m_mappedModels;
	std::vector<Object*> m_objects;

	std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;
	std::vector<Text*> m_texts;

	// Lights
	Lights m_lights;

	// Skybox
	/*std::unique_ptr<Skybox> m_skybox; // TODO: uncomment
	std::unique_ptr<CubeMapShader> m_cubeMapShader;*/ // TODO: uncomment

	//DepthShader m_depthShader; // TODO: uncomment

	// Shadow maps
	DirLightShadowMap m_dirLightShadowMap;

	// This is what the deferred renderer will render to
	std::unique_ptr<RenderableTexture> m_deferredOutputTex;
	// Particles will be rendered to a separate texture if post processing is active
	std::unique_ptr<RenderableTexture> m_particleOutputTex;

	bool m_doShadows;
	bool m_doPostProcessing;
	//PostProcessPass m_postProcessPass;

	// Camera rotation
	float m_rotation;

};
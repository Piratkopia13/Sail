#pragma once

#include "Renderer.h"
#include "../RenderableTexture.h"

class Model;
class DeferredPointLightShader;
class DeferredDirectionalLightShader;

class DeferredRenderer : public Renderer {
public:
	// Dont change, used as array indices
	enum GBuffers {
		DIFFUSE_GBUFFER = 0,
		NORMAL_GBUFFER,
		SPECULAR_GBUFFER,
		DEPTH_GBUFFER,
		NUM_GBUFFERS
	};

public:
	DeferredRenderer();
	~DeferredRenderer();

	void begin(Camera* camera) override;

	void submit(Model* model, const DirectX::SimpleMath::Matrix& modelMatrix);
	void submit(Mesh* mesh, const DirectX::SimpleMath::Matrix& modelMatrix) override;
	void setLightSetup(LightSetup* lightSetup) override;
	void end() override;
	void present(RenderableTexture* output = nullptr) override;
	virtual void onEvent(Event& event) override;

private:
	bool onResize(WindowResizeEvent& event);

	void beginGeometryPass() const;
	void doLightPass(RenderableTexture* output);

private:
	Camera* m_camera;
	LightSetup* m_lightSetup;

	std::unique_ptr<RenderableTexture> m_gBuffers[NUM_GBUFFERS - 1];

	std::unique_ptr<Model> m_screenQuadModel;
	Model* m_pointLightVolumeModel;
	DeferredPointLightShader* m_pointLightShader;
	DeferredDirectionalLightShader* m_dirLightShader;

	// Pointers to the shader resource views used as gbuffers
	ID3D11RenderTargetView* m_gBufferRTVs[NUM_GBUFFERS];


};
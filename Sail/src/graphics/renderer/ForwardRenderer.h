#pragma once

#include "Renderer.h"

class ForwardRenderer : public Renderer {
public:
	ForwardRenderer();
	~ForwardRenderer();

	void begin(Camera* camera) override;
	void submit(Model* model, const DirectX::SimpleMath::Matrix& modelMatrix) override;
	void setLightSetup(LightSetup* lightSetup) override;
	void end() override;
	void present() override;

private:
	Camera* m_camera;
	LightSetup* m_lightSetup;

};
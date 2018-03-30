#pragma once

#include <d3d11.h>
#include <SimpleMath.h>

class Camera;
class Model;
class LightSetup;

class Renderer {
public:
	struct RenderCommand {
		Model* model;
		DirectX::SimpleMath::Matrix modelMatrix; // TODO: find out why having a const ptr here doesnt work
	};
public:
	Renderer() {};
	~Renderer() {};

	virtual void begin(Camera* camera) = 0;
	virtual void submit(Model* model, const DirectX::SimpleMath::Matrix& modelMatrix) = 0;
	virtual void setLightSetup(LightSetup* lightSetup) = 0;
	virtual void end() = 0;
	virtual void present() = 0;

protected:
	std::vector<RenderCommand> commandQueue;

};
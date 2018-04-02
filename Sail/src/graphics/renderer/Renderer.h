#pragma once

#include <d3d11.h>
#include <SimpleMath.h>

class Camera;
class Model;
class Mesh;
class LightSetup;

class Renderer {
public:
	struct RenderCommand {
		Mesh* mesh;
		DirectX::SimpleMath::Matrix transform; // TODO: find out why having a const ptr here doesnt work
	};
public:
	Renderer();
	~Renderer();

	virtual void begin(Camera* camera) = 0;
	void submit(Model* model, const DirectX::SimpleMath::Matrix& modelMatrix);
	virtual void submit(Mesh* mesh, const DirectX::SimpleMath::Matrix& modelMatrix) = 0;
	virtual void setLightSetup(LightSetup* lightSetup) = 0;
	virtual void end() = 0;
	virtual void present() = 0;

protected:
	std::vector<RenderCommand> commandQueue;

};

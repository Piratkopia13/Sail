#pragma once
#include "..//BaseComponentSystem.h"
#include <d3d12.h>
#include "Sail/api/ComputeShaderDispatcher.h"
class Model;
class ModelComponent;

class AnimationSystem final : public BaseComponentSystem {
public:
	AnimationSystem();
	~AnimationSystem();

	void update(float dt) override;
	void updateOnGPU(float dt, ID3D12GraphicsCommandList4* cmdList);

private:
	std::unique_ptr<ComputeShaderDispatcher> m_dispatcher;
	Shader* m_updateShader;
	


};
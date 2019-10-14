#pragma once
#include "..//BaseComponentSystem.h"
#include <d3d12.h>
#include "Sail/api/ComputeShaderDispatcher.h"
class Model;
class ModelComponent;
class AnimationComponent;

class AnimationSystem final : public BaseComponentSystem {
public:
	AnimationSystem();
	~AnimationSystem();

	void update(float dt) override;
	void updatePerFrame();
	void toggleInterpolation();
	const bool getInterpolation();
	void setInterpolation(const bool interpolation);

	void updateTransforms(const float dt);
	void updateMeshGPU(ID3D12GraphicsCommandList4* cmdList);
	void updateMeshCPU();

	std::vector<Entity*>& getEntities();


private:
	std::unique_ptr<ComputeShaderDispatcher> m_dispatcher;
	std::unique_ptr<InputLayout> m_inputLayout;
	Shader* m_updateShader;
	
	bool m_interpolate;
	void addTime(AnimationComponent* e, const float time);
	void interpolate(glm::mat4& res, const glm::mat4& mat1, const glm::mat4& mat2, const float w);

};
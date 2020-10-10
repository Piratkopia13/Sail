#pragma once
#include "Sail/api/shader/ComputeShaderDispatcher.h"

class DX12API;

class DX12ComputeShaderDispatcher : public ComputeShaderDispatcher {
public:
	DX12ComputeShaderDispatcher();
	~DX12ComputeShaderDispatcher();

	virtual void begin(void* cmdList = nullptr) override;
	virtual void dispatch(Shader& computeShader, const glm::vec3& threadGroupCount = glm::vec3(1, 1, 1), void* cmdList = nullptr) override;

private:
	DX12API* m_context;

};
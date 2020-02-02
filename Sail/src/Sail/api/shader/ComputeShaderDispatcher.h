#pragma once
#include "Sail/graphics/shader/Shader.h"

class ComputeShaderDispatcher {
public:
	static ComputeShaderDispatcher* Create();
	ComputeShaderDispatcher() {}
	virtual ~ComputeShaderDispatcher() {}

	virtual void begin(void* cmdList = nullptr) = 0;
	virtual void dispatch(Shader& computeShader, const glm::vec3& threadGroupCount = glm::vec3(1,1,1), void* cmdList = nullptr) = 0;

};
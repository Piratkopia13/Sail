#pragma once

#include "Sail/events/Events.h"
#include "Sail/graphics/shader/Shader.h"

class ComputeShaderDispatcher : public IEventListener {
public:
	static ComputeShaderDispatcher* Create(Shader& computeShader);
	ComputeShaderDispatcher(Shader& computeShader) : computeShader(computeShader) {}
	virtual ~ComputeShaderDispatcher() {}

	virtual void setInput(Shader::ComputeShaderInput& input) {
		this->input = &input;
	}
	virtual void dispatch(void* cmdList = nullptr) = 0;
	virtual Shader::ComputeShaderOutput& getOutput() = 0;

	virtual bool onEvent(Event& event) override { return true; };

protected:
	Shader::ComputeShaderInput* input;
	Shader& computeShader;

};
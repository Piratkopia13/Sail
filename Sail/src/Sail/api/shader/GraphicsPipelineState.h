#pragma once

class GraphicsPipelineState {
public:
	static GraphicsPipelineState* GraphicsPipelineState::Create();
	GraphicsPipelineState() {};
	virtual ~GraphicsPipelineState() {};

	virtual void bind() const = 0;

protected:

private:

};

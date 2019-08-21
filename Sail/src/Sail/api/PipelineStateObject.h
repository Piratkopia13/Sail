#pragma once

class PipelineStateObject {
public:
	PipelineStateObject() {};
	virtual ~PipelineStateObject() {};

	virtual void enable() const = 0;

protected:

private:

};
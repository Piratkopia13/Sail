#pragma once

#include "Sail/api/Renderer.h"
#include <glm/glm.hpp>
#include "../DX12API.h"

//#define MULTI_THREADED_COMMAND_RECORDING

class DX12ForwardRenderer : public Renderer {
public:
	DX12ForwardRenderer();
	~DX12ForwardRenderer();

	void present(RenderableTexture* output = nullptr) override;

private:
	static const int nRecordThreads = 2;

	DX12API* m_context;
	DX12API::Command m_command[nRecordThreads];

	void RecordCommands(const int threadID, const int frameIndex);
};
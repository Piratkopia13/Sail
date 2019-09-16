#pragma once

#include "Sail/api/Renderer.h"
#include <glm/glm.hpp>
#include "../DX12API.h"

#define MULTI_THREADED_COMMAND_RECORDING
//#define DEBUG_MULTI_THREADED_COMMAND_RECORDING

class DX12ForwardRenderer : public Renderer {
public:
	DX12ForwardRenderer();
	~DX12ForwardRenderer();

	void present(RenderableTexture* output = nullptr) override;

private:
	static const int MAX_RECORD_THREADS = 4;
	static const int MIN_COMMANDS_PER_THREAD = 20;

	DX12API* m_context;
	DX12API::Command m_command[MAX_RECORD_THREADS];

	void recordCommands(const int threadID, const int frameIndex, const int start, const int nCommands, size_t oobMax, int nThreads);
};
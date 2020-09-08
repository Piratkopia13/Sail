#pragma once

#include "Sail/api/GraphicsAPI.h"
#include "Sail/graphics/geometry/Model.h"
#include "Sail/graphics/camera/Camera.h"
#include "Sail/graphics/shader/BindShader.h"
#include "InputLayout.h"
#include "../RenderableTexture.h"

class Shader;

class PipelineStateObject {
public:
	static PipelineStateObject* CurrentlyBoundPSO;

	class PSOSettings {
	public:
		bool wireframe = false;
		GraphicsAPI::Culling cullMode = GraphicsAPI::NO_CULLING;
		GraphicsAPI::DepthMask depthMask = GraphicsAPI::NO_MASK;
		GraphicsAPI::Blending blendMode = GraphicsAPI::NO_BLENDING;
		unsigned int numRenderTargets = 1;
		std::unordered_map<unsigned int, ResourceFormat::TextureFormat> rtFormats;
	};

public:
	static PipelineStateObject* Create(Shader* shader, unsigned int attributesHash);
	PipelineStateObject(Shader* shader, unsigned int attributesHash);
	virtual ~PipelineStateObject() = 0;

	// Returns false if already bound
	virtual bool bind(void* cmdList = nullptr, uint32_t frameIndex = -1);
	virtual void unbind();
	
protected:
	// Binds shader resources using specified arguments
	bool bindInternal(void* cmdList, bool forceIfBound, uint32_t frameIndex);

protected:
	Shader* shader;
	std::shared_ptr<InputLayout> inputLayout;
	PSOSettings settings;

};
#pragma once

#include <glm/glm.hpp>
#include "Sail/api/Renderer.h"
#include "../DX12API.h"
#include "../DXR/DXRBase.h"
#include "../resources/DX12RenderableTexture.h"
#include "Sail/graphics/shader/postprocess/BilateralBlurHorizontal.h"
#include "Sail/graphics/shader/postprocess/BilateralBlurVertical.h"
#include "API/DX12/DX12ComputeShaderDispatcher.h"
#include "API/DX12/DX12Mesh.h"

class ShadePassShader;

class DX12RaytracingRenderer : public Renderer {
public:
	DX12RaytracingRenderer(DX12RenderableTexture** inputs);
	~DX12RaytracingRenderer();

	void present(PostProcessPipeline* postProcessPipeline = nullptr, RenderableTexture* output = nullptr) override;
	virtual void begin(Camera* camera) override;

	virtual bool onEvent(const Event& event) override;
	virtual void submit(Mesh* mesh, const glm::mat4& modelMatrix, RenderFlag flags, int teamColorID, bool castShadows) override;
	virtual void submitMetaball(RenderCommandType type, Material* material, const glm::vec3& pos, RenderFlag flags) override;
	virtual void submitDecal(const glm::vec3& pos, const glm::mat3& rot, const glm::vec3& halfSize) override;
	virtual void submitWaterPoint(const glm::vec3& pos) override;
	virtual bool checkIfOnWater(const glm::vec3& pos) override;

	virtual void setTeamColors(const std::vector<glm::vec3>& teamColors);
	virtual void updateMetaballAABB();

	void setGBufferInputs(DX12RenderableTexture** inputs);
	DXRBase* getDXRBase();

private:
	DX12RenderableTexture* runDenoising(ID3D12GraphicsCommandList4* cmdList);
	DX12RenderableTexture* runShading(ID3D12GraphicsCommandList4* cmdList, DX12RenderableTexture* shadows);
	bool onResize(const WindowResizeEvent& event);

private:
	DX12API* m_context;
	DXRBase m_dxr;
	DX12API::Command m_commandDirectShading;
	DX12API::Command m_commandDirectCopy;
	DX12API::Command m_commandCompute;
	DX12API::Command m_commandComputePostProcess;

	DX12RenderableTexture** m_gbufferTextures;
	Texture* m_brdfTexture;

	DXRBase::BounceOutput m_outputTextures;
	std::unique_ptr<DX12RenderableTexture> m_outputBloomTexture;
	std::unique_ptr<DX12RenderableTexture> m_shadedOutput;
	std::unique_ptr<DX12RenderableTexture> m_shadowsLastFrame;
	std::unique_ptr<Model> m_fullscreenModel;
	ShadePassShader* m_shadeShader;

	DX12ComputeShaderDispatcher m_computeShaderDispatcher;

	std::vector<DXRBase::Metaball> m_metaballs;
	D3D12_RAYTRACING_AABB m_nextMetaballAabb;

	// Decals
	DXRShaderCommon::DecalData m_decals[MAX_DECALS];
	size_t m_currNumDecals;
};

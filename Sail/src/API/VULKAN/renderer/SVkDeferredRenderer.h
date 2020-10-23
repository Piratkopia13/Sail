#pragma once

#include "Sail/api/Renderer.h"
#include "Sail/events/Events.h"
#include "Sail/graphics/material/TexturesMaterial.h"
#include "../SVkAPI.h"
#include "../resources/SVkRenderableTexture.h"
#include "Sail/graphics/SSAO.h"

class SVkDeferredRenderer : public Renderer, public IEventListener {
public:
	static const unsigned int NUM_GBUFFERS = 4;

	struct GBufferTextures {
		std::unique_ptr<SVkRenderableTexture> positions;
		std::unique_ptr<SVkRenderableTexture> normals;
		std::unique_ptr<SVkRenderableTexture> albedo;
		std::unique_ptr<SVkRenderableTexture> mrao;
		VkImageView depthView = {};
	};

public:
	SVkDeferredRenderer();
	~SVkDeferredRenderer();

	virtual void* present(Renderer::PresentFlag flags, void* skippedPrepCmdList = nullptr) override;
	virtual void* getDepthBuffer() override;

	virtual bool onEvent(Event& event) override;

	static const GBufferTextures& GetGBuffers();

private:
	void createGeometryRenderPass();
	void createShadingRenderPass();
	void createGeometryFramebuffers();
	void createSSAORenderPass();
	void createSSAOFramebuffers();

	const VkCommandBuffer& runFramePreparation();
	void runGeometryPass(const VkCommandBuffer& cmd);
	void runSSAO(const VkCommandBuffer& cmd);
	void runShadingPass(const VkCommandBuffer& cmd);
	void runFrameExecution(const VkCommandBuffer& cmd);


private:
	SVkAPI* m_context;
	SVkAPI::Command m_command;

	VkRenderPass m_geometryRenderPass;
	static GBufferTextures sGBuffers;
	std::vector<VkFramebuffer> m_geometryFramebuffers;

	VkRenderPass m_shadingRenderPass;
	std::shared_ptr<Mesh> m_screenQuadMesh;
	TexturesMaterial m_shadingPassMaterial;
	Texture* m_brdfLutTexture;

	std::unique_ptr<SSAO> m_ssao;
	VkRenderPass m_ssaoRenderPass;
	std::vector<VkFramebuffer> m_ssaoFramebuffers;
	SVkRenderableTexture* m_ssaoShadingTexture;

	unsigned int m_width, m_height;

};
#pragma once

#include "Sail/api/Renderer.h"
#include "Sail/events/Events.h"
#include "Sail/graphics/material/TexturesMaterial.h"
#include "../SVkAPI.h"
#include "../resources/SVkRenderableTexture.h"

class SVkDeferredRenderer : public Renderer, public IEventListener {
public:
	static const unsigned int NUM_GBUFFERS = 4;

public:
	SVkDeferredRenderer();
	~SVkDeferredRenderer();

	virtual void* present(Renderer::PresentFlag flags, void* skippedPrepCmdList = nullptr) override;
	virtual void* getDepthBuffer() override;

	virtual bool onEvent(Event& event) override;

private:
	struct GBufferTextures {
		std::unique_ptr<SVkRenderableTexture> positions;
		std::unique_ptr<SVkRenderableTexture> normals;
		std::unique_ptr<SVkRenderableTexture> albedo;
		std::unique_ptr<SVkRenderableTexture> mrao;
		VkImageView depthView;
	};

	void createGeometryRenderPass();
	void createShadingRenderPass();
	void createFramebuffers();

	const VkCommandBuffer& runFramePreparation();
	void runGeometryPass(const VkCommandBuffer& cmd);
	void runShadingPass(const VkCommandBuffer& cmd);
	void runFrameExecution(const VkCommandBuffer& cmd);


private:
	SVkAPI* m_context;
	SVkAPI::Command m_command;

	VkRenderPass m_geometryRenderPass;
	GBufferTextures m_gbuffers;
	std::vector<VkFramebuffer> m_frameBuffers;

	VkRenderPass m_shadingRenderPass;
	std::unique_ptr<Model> m_screenQuadModel;
	TexturesMaterial m_shadingPassMaterial;
	Texture* m_brdfLutTexture;

	unsigned int m_width, m_height;

};
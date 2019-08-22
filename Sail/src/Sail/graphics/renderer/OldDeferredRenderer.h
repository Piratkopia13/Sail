#pragma once

#include <d3d11.h>
#include <SimpleMath.h>
#include <memory>
#include "../RenderableTexture.h"
#include "../geometry/Model.h"
#include "../models/FbxModel.h"
#include "../shader/basic/SimpleTextureShader.h"
#include "../shader/deferred/DeferredGeometryShader.h"
#include "../shader/deferred/DeferredDirectionalLightShader.h"
#include "../shader/deferred/DeferredPointLightShader.h"
#include "../camera/PerspectiveCamera.h"
#include "../shadows/DirLightShadowMap.h"

class DeferredRenderer {

public:
	// Dont change, used as array indices
	enum GBuffers {
		DIFFUSE_GBUFFER = 0,
		NORMAL_GBUFFER,
		SPECULAR_GBUFFER,
		DEPTH_GBUFFER,
		NUM_GBUFFERS
	};

public:
	DeferredRenderer();
	~DeferredRenderer();

	void beginGeometryPass(Camera& camera, ID3D11RenderTargetView* const lightPassRTV, ID3D11DepthStencilView* const dsv = nullptr);
	void beginLightDepthPass(ID3D11DepthStencilView* const dsv);
	void doLightPass(LightSetup& lights, Camera& cam, DirLightShadowMap* dlShadowMap = nullptr);
	void resize(int width, int height);
	ID3D11ShaderResourceView** getGBufferSRV(UINT index);
	RenderableTexture* getGBufferRenderableTexture(UINT index);
	ID3D11DepthStencilView* const getDSV() const;

private:
	void createFullscreenQuad();

private:

	std::unique_ptr<RenderableTexture> m_gBuffers[NUM_GBUFFERS - 1];
	ID3D11RenderTargetView* m_rtvs[NUM_GBUFFERS];
	ID3D11ShaderResourceView* m_srvs[NUM_GBUFFERS];
	ID3D11ShaderResourceView* m_dsvSrv;
	ID3D11DepthStencilView* m_dsv;
	std::unique_ptr<Model> m_fullScreenPlane;

	// Light volumes
	std::unique_ptr<FbxModel> m_pointLightVolume;

};
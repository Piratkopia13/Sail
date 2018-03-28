#pragma once

#include "PostProcessStage.h"
#include "../../shader/ShaderSet.h"
#include "../../shader/component/ConstantBuffer.h"
#include "../../shader/component/Sampler.h"
#include "../../../Application.h"

class DOFStage : public PostProcessStage {
public:
	DOFStage(UINT width, UINT height, Model* fullScreenQuad);
	virtual ~DOFStage();

	void setDepthSRV(ID3D11ShaderResourceView** depthSRV);
	void run(RenderableTexture& inputTexture);
	void resize(UINT width, UINT height);

private:
	void createOutputUAV();

private:
	std::unique_ptr<ComputeShader> m_CS;

	struct CBuffer {
		float zNear;
		float zFar;
		float padding[2];
	};

	ID3D11UnorderedAccessView* m_outputUAV;
	std::unique_ptr<ShaderComponent::ConstantBuffer> m_cBuffer;
	ID3D11ShaderResourceView** m_depthSRV;
};

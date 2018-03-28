#include "HGaussianBlurStage.h"

HGaussianBlurStage::HGaussianBlurStage(UINT width, UINT height, Model* fullscreenQuad)
	: PostProcessStage(width, height, fullscreenQuad)
{
	// Set up constant buffer
	CBuffer pixelSize = { 1.f / width, 1.f / height };
	m_cBuffer = std::unique_ptr<ShaderComponent::ConstantBuffer>(new ShaderComponent::ConstantBuffer(&pixelSize, sizeof(pixelSize)));

	// Set up sampler
	m_sampler = std::make_unique<ShaderComponent::Sampler>(D3D11_TEXTURE_ADDRESS_MIRROR_ONCE, D3D11_FILTER_MIN_MAG_MIP_LINEAR);

	// Compile and assign shaders
	auto vsBlob = ShaderSet::compileShader(L"postprocess/GaussianBlurPS.hlsl", "VSMain", "vs_5_0");
	auto psBlob = ShaderSet::compileShader(L"postprocess/GaussianBlurPS.hlsl", "PSHorizontal", "ps_5_0");
	m_VS = std::make_unique<VertexShader>(vsBlob);
	m_PS = std::make_unique<PixelShader>(psBlob);
	Memory::safeRelease(vsBlob);
	Memory::safeRelease(psBlob);

}

HGaussianBlurStage::~HGaussianBlurStage() {
}

void HGaussianBlurStage::run(RenderableTexture& inputTexture) {
	// TODO: remove

	//Application* app = Application::getInstance();
	//ID3D11DeviceContext* con = app->getDXManager()->getDeviceContext();

	//// Bind shaders
	//m_VS->bind();
	//m_PS->bind();

	//// Bind sampler
	//m_sampler->bind(ShaderComponent::PS);
	//// Bind cbuffer
	//m_cBuffer->bind(ShaderComponent::PS);

	//// Set output target
	//OutputTexture.begin();

	//// Bind the input texture to the pixel shader
	//con->PSSetShaderResources(0, 1, inputTexture.getColorSRV());

	//// Bind vertex buffer
	//UINT stride = sizeof(PostProcessStage::Vertex);
	//UINT offset = 0;
	//con->IASetVertexBuffers(0, 1, FullscreenQuad->getVertexBuffer(), &stride, &offset);

	//// Bind index buffer if one exitsts
	//auto iBuffer = FullscreenQuad->getIndexBuffer();
	//if (iBuffer)
	//	con->IASetIndexBuffer(iBuffer, DXGI_FORMAT_R32_UINT, 0);

	//// Set topology
	//con->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//// Draw horizontal
	//if (iBuffer)
	//	con->DrawIndexed(FullscreenQuad->getNumIndices(), 0, 0);
	//else
	//	con->Draw(FullscreenQuad->getNumVertices(), 0);

	//ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
	//Application::getInstance()->getDXManager()->getDeviceContext()->PSSetShaderResources(0, 1, nullSRV);

}

void HGaussianBlurStage::resize(UINT width, UINT height) {
	PostProcessStage::resize(width, height);
	// Update constant buffer
	CBuffer pixelSize = { 1.f / width, 1.f / height };
	m_cBuffer->updateData(&pixelSize, sizeof(pixelSize));
}

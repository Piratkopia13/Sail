#include "GaussianDOFStage.h"

GaussianDOFStage::GaussianDOFStage(UINT width, UINT height, Model* fullScreenQuad)
	: PostProcessStage(width, height, fullScreenQuad, D3D11_BIND_UNORDERED_ACCESS) {

	// Set up sampler
	m_sampler = std::make_unique<ShaderComponent::Sampler>(D3D11_TEXTURE_ADDRESS_MIRROR, D3D11_FILTER_MIN_MAG_MIP_LINEAR);

	// Set up constant buffer
	CBuffer data = { 0.1f, 1000.f, 5.f, 10.f };
	m_cBuffer = std::unique_ptr<ShaderComponent::ConstantBuffer>(new ShaderComponent::ConstantBuffer(&data, sizeof(data)));

	// Compile and assign shaders
	auto vsBlob = ShaderSet::compileShader(L"postprocess/SimpleDepthOfField.hlsl", "VSMain", "vs_5_0");
	auto psBlob = ShaderSet::compileShader(L"postprocess/SimpleDepthOfField.hlsl", "PSMain", "ps_5_0");
	m_VS = std::make_unique<VertexShader>(vsBlob);
	m_PS = std::make_unique<PixelShader>(psBlob);
	Memory::safeRelease(vsBlob);
	Memory::safeRelease(psBlob);

}

GaussianDOFStage::~GaussianDOFStage() {
}

void GaussianDOFStage::setDepthSRV(ID3D11ShaderResourceView** depthSRV) {
	m_depthSRV = depthSRV;
}

void GaussianDOFStage::setBlurredSRV(ID3D11ShaderResourceView** blurredSRV) {
	m_blurredSRV = blurredSRV;
}

void GaussianDOFStage::run(RenderableTexture& inputTexture) {
	// TODO: fix

	//Application* app = Application::getInstance();
	//ID3D11DeviceContext* con = app->getDXManager()->getDeviceContext();

	//// Bind shaders
	//m_VS->bind();
	//m_PS->bind();

	//// Bind sampler
	//m_sampler->bind();

	//// Bind constant buffer
	//m_cBuffer->bind(ShaderComponent::PS);

	//// Set output target
	//OutputTexture.begin();

	//// Bind the input textures to the pixel shader
	//con->PSSetShaderResources(0, 1, inputTexture.getColorSRV());
	//con->PSSetShaderResources(1, 1, m_blurredSRV);
	//con->PSSetShaderResources(2, 1, m_depthSRV);

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

	//ID3D11ShaderResourceView* nullSRV[3] = { nullptr, nullptr, nullptr };
	//Application::getInstance()->getDXManager()->getDeviceContext()->PSSetShaderResources(0, 3, nullSRV);

}

void GaussianDOFStage::setFocus(float focusDistance, float focusWidth) {
	CBuffer data = { 0.1f, 1000.f, focusDistance, focusWidth };
	m_cBuffer->updateData(&data, sizeof(data));
}

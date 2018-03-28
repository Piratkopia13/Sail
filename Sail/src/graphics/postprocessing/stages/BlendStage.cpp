#include "BlendStage.h"

BlendStage::BlendStage(UINT width, UINT height, Model* fullScreenQuad)
	: PostProcessStage(width, height, fullScreenQuad, D3D11_BIND_UNORDERED_ACCESS) {

	// Set up sampler
	m_sampler = std::make_unique<ShaderComponent::Sampler>(D3D11_TEXTURE_ADDRESS_MIRROR, D3D11_FILTER_MIN_MAG_MIP_LINEAR);

	// Set up constant buffer
	CBuffer data = { 0.5f };
	m_cBuffer = std::unique_ptr<ShaderComponent::ConstantBuffer>(new ShaderComponent::ConstantBuffer(&data, sizeof(data)));

	// Compile and assign shaders
	auto vsBlob = ShaderSet::compileShader(L"postprocess/BlendShader.hlsl", "VSMain", "vs_5_0");
	auto psBlob = ShaderSet::compileShader(L"postprocess/BlendShader.hlsl", "PSMain", "ps_5_0");
	m_VS = std::make_unique<VertexShader>(vsBlob);
	m_PS = std::make_unique<PixelShader>(psBlob);
	Memory::safeRelease(vsBlob);
	Memory::safeRelease(psBlob);

}

BlendStage::~BlendStage() {}

void BlendStage::setBlendInput(ID3D11ShaderResourceView** blendSRV, float blendFactor) {
	m_blendSRV = blendSRV;
	CBuffer data = { blendFactor };
	m_cBuffer->updateData(&data, sizeof(data));
}

void BlendStage::run(RenderableTexture& inputTexture) {
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
	//con->PSSetShaderResources(1, 1, m_blendSRV);

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

	//ID3D11ShaderResourceView* nullSRV[2] = { nullptr, nullptr };
	//Application::getInstance()->getDXManager()->getDeviceContext()->PSSetShaderResources(0, 2, nullSRV);

}
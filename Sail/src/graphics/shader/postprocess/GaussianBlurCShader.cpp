#include "GaussianBlurCShader.h"
#include "../../geometry/Model.h"
#include "../../geometry/Material.h"

using namespace DirectX::SimpleMath;

GaussianBlurCShaderPoo::GaussianBlurCShaderPoo()
	: m_horInputSRV(nullptr)
	, m_vertInputSRV(nullptr)
	, m_horPassUAV(nullptr)
	, m_vertPassUAV(nullptr)
	, m_fullScreenQuadModel(nullptr)
{
// 
// 	auto window = Application::getInstance()->getWindow();
// 	UINT width = window->getWindowWidth();
// 	UINT height = window->getWindowHeight();
// 
// 	

	// Set up constant buffer
	CBuffer pixelSize = { 1.f };
	m_cBuffer = std::unique_ptr<ShaderComponent::ConstantBuffer>(new ShaderComponent::ConstantBuffer(&pixelSize, sizeof(pixelSize)));

	// Set up sampler
	m_sampler = std::make_unique<ShaderComponent::Sampler>(D3D11_TEXTURE_ADDRESS_MIRROR_ONCE, D3D11_FILTER_MIN_MAG_MIP_LINEAR);

	// Compile shaders
	auto vsBlob = compileShader(L"postprocess/GaussianBlurPS.hlsl", "VSMain", "vs_5_0");
	auto psHorBlob = compileShader(L"postprocess/GaussianBlurPS.hlsl", "PSHorizontal", "ps_5_0");
	auto psVertBlob = compileShader(L"postprocess/GaussianBlurPS.hlsl", "PSVertical", "ps_5_0");

	auto csHorBlob = compileShader(L"postprocess/GaussianBlurCS.hlsl", "CSHorizontal", "cs_5_0");
	auto csVertBlob = compileShader(L"postprocess/GaussianBlurCS.hlsl", "CSVertical", "cs_5_0");

	setVertexShader(vsBlob);
	setPixelShader(psHorBlob);
	m_vertPS = std::make_unique<PixelShader>(psVertBlob);

	// Set both the compute shaders
	ID3D10Blob** csBlobs = new ID3D10Blob*[2];
	csBlobs[0] = csHorBlob;
	csBlobs[1] = csVertBlob;
	setComputeShaders(csBlobs, 2);

	// Done with the blobs, release them
	Memory::safeRelease(csHorBlob);
	Memory::safeRelease(csVertBlob);
	Memory::safeRelease(vsBlob);
	Memory::safeRelease(psHorBlob);
	Memory::safeRelease(psVertBlob);
	delete[] csBlobs;

	createBuffers();

}

GaussianBlurCShaderPoo::~GaussianBlurCShaderPoo() {
	Memory::safeRelease(m_horPassUAV);
	Memory::safeRelease(m_vertPassUAV);
}

void GaussianBlurCShaderPoo::updateConstantBuffer() const {

}

void GaussianBlurCShaderPoo::bind() {
	ShaderSet::bind();
}

void GaussianBlurCShaderPoo::createBuffers() {

}

void GaussianBlurCShaderPoo::setInputSRV(ID3D11ShaderResourceView** srv) {
	m_horInputSRV = srv;
}

void GaussianBlurCShaderPoo::setHorPassUAV(ID3D11Texture2D* tex) {
	auto* dev = Application::getInstance()->getDXManager()->getDevice();
	D3D11_TEXTURE2D_DESC texDesc;
	tex->GetDesc(&texDesc);

	Memory::safeRelease(m_horPassUAV);

	D3D11_UNORDERED_ACCESS_VIEW_DESC descView;
	ZeroMemory(&descView, sizeof(descView));
	descView.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	descView.Buffer.FirstElement = 0;
	descView.Buffer.NumElements = texDesc.Width * texDesc.Height;
	descView.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;

	ThrowIfFailed(dev->CreateUnorderedAccessView(tex, &descView, &m_horPassUAV));
}

void GaussianBlurCShaderPoo::setOutputTexture(RenderableTexture* tex) {
	m_outputTex = tex;

	auto* dev = Application::getInstance()->getDXManager()->getDevice();
	D3D11_TEXTURE2D_DESC texDesc;
	m_outputTex->getTexture2D()->GetDesc(&texDesc);

	Memory::safeRelease(m_vertPassUAV);

	D3D11_UNORDERED_ACCESS_VIEW_DESC descView;
	ZeroMemory(&descView, sizeof(descView));
	descView.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	descView.Buffer.FirstElement = 0;
	descView.Buffer.NumElements = texDesc.Width * texDesc.Height;
	descView.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;

	ThrowIfFailed(dev->CreateUnorderedAccessView(m_outputTex->getTexture2D(), &descView, &m_vertPassUAV));
}

void GaussianBlurCShaderPoo::resize(int width, int height) {
	m_middleTex->resize(width, height);
	m_vertInputSRV = m_middleTex->getColorSRV();
	setHorPassUAV(m_middleTex->getTexture2D());
}

void GaussianBlurCShaderPoo::setTextureSize(UINT width, UINT height) {
	m_texWidth = width;
	m_texHeight = height;

	UINT windowWidth = Application::getInstance()->getWindow()->getWindowWidth();
	
	CBuffer pixelSize = { windowWidth / (float)width };
	m_cBuffer->updateData(&pixelSize, sizeof(pixelSize));

	// Set up the "middle" texture used betwwen the two passes
	m_middleTex = std::unique_ptr<RenderableTexture>(new RenderableTexture(1U, width, height, true, false, D3D11_BIND_UNORDERED_ACCESS));
	m_vertInputSRV = m_middleTex->getColorSRV();
	setHorPassUAV(m_middleTex->getTexture2D());
}

void GaussianBlurCShaderPoo::setFullScreenQuadModel(Model* model) {
	m_fullScreenQuadModel = model;
}

void GaussianBlurCShaderPoo::draw(bool bindFirst) {
	// TODO: remove

	//m_middleTex->clear({ 0.0, 0.0, 0.0, 0.0 });

	//Application* app = Application::getInstance();
	//ID3D11DeviceContext* con = app->getDXManager()->getDeviceContext();

	//if (true) {
	//	// Pixel shader method
	//	bind();

	//	m_middleTex->begin();

	//	m_sampler->bind(ShaderComponent::PS);

	//	// Bind the texture if it exists
	//	UINT numTextures;
	//	auto tex = m_fullScreenQuadModel->getMaterial()->getTextures(numTextures);
	//	if (tex)
	//		con->PSSetShaderResources(0, numTextures, tex);

	//	// Bind vertex buffer
	//	UINT stride = sizeof(GaussianBlurCShaderPoo::Vertex);
	//	UINT offset = 0;
	//	con->IASetVertexBuffers(0, 1, m_fullScreenQuadModel->getVertexBuffer(), &stride, &offset);

	//	// Bind index buffer if one exitsts
	//	auto iBuffer = m_fullScreenQuadModel->getIndexBuffer();
	//	if (iBuffer)
	//		con->IASetIndexBuffer(iBuffer, DXGI_FORMAT_R32_UINT, 0);

	//	// Set topology
	//	con->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//	// Draw horizontal
	//	if (iBuffer)
	//		con->DrawIndexed(m_fullScreenQuadModel->getNumIndices(), 0, 0);
	//	else
	//		con->Draw(m_fullScreenQuadModel->getNumVertices(), 0);

	//	// Draw vertical
	//	m_vertPS->bind();
	//	// Bind the horizontal pass output as this input
	//	//app->getDXManager()->renderToBackBuffer();
	//	m_outputTex->begin();
	//	con->PSSetShaderResources(0, 1, m_middleTex->getColorSRV());

	//	if (iBuffer)
	//		con->DrawIndexed(m_fullScreenQuadModel->getNumIndices(), 0, 0);
	//	else
	//		con->Draw(m_fullScreenQuadModel->getNumVertices(), 0);

	//	ID3D11ShaderResourceView* nullSRV[5] = { nullptr, nullptr, nullptr, nullptr, nullptr };
	//	Application::getInstance()->getDXManager()->getDeviceContext()->PSSetShaderResources(0, 5, nullSRV);


	//}
	//else {

	//	// Compute shader method

	//	UINT initCounts = 1;
	//
	//	ID3D11UnorderedAccessView* pNullUAV = nullptr;
	//	ID3D11ShaderResourceView* pNullSRV = nullptr;
	//
	//	// Bind the sampler
	//	m_sampler->bind(ShaderComponent::CS, 0);
	//	// Bind the constant buffer
	//	m_cBuffer->bind(ShaderComponent::CS, 0);
	//
	//	//////////////////////////////////////////////////////////////////////////
	//	//							DO HORIZONTAL BLUR PASS
	//	//////////////////////////////////////////////////////////////////////////
	//	bindCS(0);
	//
	//	//con->CSSetUnorderedAccessViews(0, 1, &m_horPassUAV, nullptr);
	//  	con->CSSetUnorderedAccessViews(0, 1, &m_vertPassUAV, nullptr);
	//	con->CSSetShaderResources(0, 1, m_horInputSRV);
	//
	//	// Do as many "full" dispatches as possible
	//	UINT32 numFullPases = static_cast<UINT32>(ceilf(m_texWidth / 1024.0f));
	//	con->Dispatch(1, m_texHeight, numFullPases);
	//
	//
	//	//////////////////////////////////////////////////////////////////////////
	//	//							DO VERTICAL BLUR PASS
	//	//////////////////////////////////////////////////////////////////////////
	//	//bindCS(1);
	//
	//	//con->CSSetUnorderedAccessViews(0, 1, &m_vertPassUAV, nullptr);
	//	//con->CSSetShaderResources(0, 1, m_vertInputSRV);
	//
	//	//// Do as many "full" dispatches as possible
	//	//con->Dispatch(m_texWidth, 1, numFullPases);
	//
	//
	//	// Unbind resources from shader
	//	con->CSSetUnorderedAccessViews(0, 1, &pNullUAV, nullptr);
	//	con->CSSetShaderResources(0, 1, &pNullSRV);

	//}

}

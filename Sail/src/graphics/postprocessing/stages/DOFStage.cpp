#include "DOFStage.h"



DOFStage::DOFStage(UINT width, UINT height, Model* fullScreenQuad) 
	: PostProcessStage(width, height, fullScreenQuad, D3D11_BIND_UNORDERED_ACCESS)
{
	// Set up constant buffer
	CBuffer camPlanes = { 0.1f, 1000.f };
	m_cBuffer = std::unique_ptr<ShaderComponent::ConstantBuffer>(new ShaderComponent::ConstantBuffer(&camPlanes, sizeof(camPlanes)));

	// Compile and assign shaders
	auto csBlob = ShaderSet::compileShader(L"postprocess/DepthOfField.hlsl", "CSMain", "cs_5_0");
	m_CS = std::make_unique<ComputeShader>(csBlob);
	Memory::safeRelease(csBlob);

	m_outputUAV = nullptr;
	createOutputUAV();
}

DOFStage::~DOFStage() {
	Memory::safeRelease(m_outputUAV);
}

void DOFStage::setDepthSRV(ID3D11ShaderResourceView** depthSRV) {
	m_depthSRV = depthSRV;
}

void DOFStage::run(RenderableTexture& inputTexture) {
	Application* app = Application::getInstance();
	ID3D11DeviceContext* con = app->getDXManager()->getDeviceContext();

	OutputTexture.clear({0.f, 0.f, 0.f, 0.f});

	// Bind compute shader
	m_CS->bind();

	// Bind constant buffer
	m_cBuffer->bind(ShaderComponent::CS);

	con->CSSetUnorderedAccessViews(0, 1, &m_outputUAV, nullptr);
	con->CSSetShaderResources(0, 1, inputTexture.getColorSRV());
	con->CSSetShaderResources(1, 1, m_depthSRV);

	// Do as many "full" dispatches as possible
	UINT32 numFullPases = static_cast<UINT32>(ceilf(Width / 1024.0f));
	con->Dispatch(1, Height, numFullPases);


	// Unbind resources from shader
	ID3D11UnorderedAccessView* pNullUAV = nullptr;
	ID3D11ShaderResourceView* pNullSRV = nullptr;
	con->CSSetUnorderedAccessViews(0, 1, &pNullUAV, nullptr);
	con->CSSetShaderResources(0, 1, &pNullSRV);
}

void DOFStage::resize(UINT width, UINT height) {
	PostProcessStage::resize(width, height);
	createOutputUAV();
}

void DOFStage::createOutputUAV() {
	// Release old uav if set
	Memory::safeRelease(m_outputUAV);

	// Create the output UAV
	ID3D11Device* dev = Application::getInstance()->getDXManager()->getDevice();

	D3D11_UNORDERED_ACCESS_VIEW_DESC descView;
	ZeroMemory(&descView, sizeof(descView));
	descView.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	descView.Buffer.FirstElement = 0;
	descView.Buffer.NumElements = Width * Height;
	descView.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;

	ThrowIfFailed(dev->CreateUnorderedAccessView(OutputTexture.getTexture2D(), &descView, &m_outputUAV));
}
#include "pch.h"
#include "DX12Utils.h"
#include "Sail/utils/Utils.h"

#include <comdef.h>
#include "resources/DX12ATexture.h"

void DX12Utils::checkDeviceRemovalReason(ID3D12Device5* device, HRESULT hr) {
#ifdef DEVELOPMENT
	if (FAILED(hr)) {
		SAIL_LOG_ERROR("Device Error message can be found in output!");
		_com_error err(hr);
		std::cout << err.ErrorMessage() << std::endl;
		OutputDebugStringW(err.ErrorMessage());

		hr = device->GetDeviceRemovedReason();
		_com_error err2(hr);
		std::cout << err2.ErrorMessage() << std::endl;
		OutputDebugStringW(err2.ErrorMessage());

		return;
	}
#endif // DEVELOPMENT
}

void DX12Utils::UpdateDefaultBufferData(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, const void* data, UINT64 byteSize, UINT64 offset, ID3D12Resource* defaultBuffer, ID3D12Resource** uploadBuffer) {
	// TODO: make this method useful

	D3D12_RESOURCE_DESC bufferDesc{};
	bufferDesc.Width = byteSize;
	bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	bufferDesc.Height = 1;
	bufferDesc.MipLevels = 1;
	bufferDesc.DepthOrArraySize = 1;
	bufferDesc.SampleDesc.Count = 1;
	bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	ThrowIfFailed(device->CreateCommittedResource(
		&sUploadHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&bufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(uploadBuffer)));

	// Put in barriers in order to schedule for the data to be copied
	// to the default buffer resource.
	SetResourceTransitionBarrier(cmdList, defaultBuffer, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_DEST);

	// Prepare the data to be uploaded to the GPU
	BYTE* pData;
	ThrowIfFailed((*uploadBuffer)->Map(0, NULL, reinterpret_cast<void**>(&pData)));
	memcpy(pData, data, byteSize);
	(*uploadBuffer)->Unmap(0, NULL);

	// Copy the data from the uploadBuffer to the defaultBuffer
	cmdList->CopyBufferRegion(defaultBuffer, offset, *uploadBuffer, 0, byteSize);

	// "Remove" the resource barrier
	SetResourceTransitionBarrier(cmdList, defaultBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
}

ID3D12Resource* DX12Utils::CreateBuffer(ID3D12Device* device, UINT64 size, D3D12_RESOURCE_FLAGS flags, D3D12_RESOURCE_STATES initState, const D3D12_HEAP_PROPERTIES& heapProps, D3D12_RESOURCE_DESC* bufDesc) {

	D3D12_RESOURCE_DESC newBufDesc = {};
	if (!bufDesc) {
		newBufDesc.Alignment = 0;
		newBufDesc.DepthOrArraySize = 1;
		newBufDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		newBufDesc.Flags = flags;
		newBufDesc.Format = DXGI_FORMAT_UNKNOWN;
		newBufDesc.Height = 1;
		newBufDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		newBufDesc.MipLevels = 1;
		newBufDesc.SampleDesc.Count = 1;
		newBufDesc.SampleDesc.Quality = 0;
		newBufDesc.Width = size;

		bufDesc = &newBufDesc;
	}

	ID3D12Resource* pBuffer = nullptr;
	auto hr = device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, bufDesc, initState, nullptr, IID_PPV_ARGS(&pBuffer));
	if (FAILED(hr)) {
		_com_error err(hr);
		std::cout << err.ErrorMessage() << std::endl;

		hr = device->GetDeviceRemovedReason();
		_com_error err2(hr);
		std::cout << err2.ErrorMessage() << std::endl;
	}
	return pBuffer;
}

void DX12Utils::SetResourceTransitionBarrier(ID3D12GraphicsCommandList* commandList, ID3D12Resource* resource, D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter, UINT subResource) {
	D3D12_RESOURCE_BARRIER barrierDesc = {};

	barrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrierDesc.Transition.pResource = resource;
	barrierDesc.Transition.Subresource = subResource;
	barrierDesc.Transition.StateBefore = stateBefore;
	barrierDesc.Transition.StateAfter = stateAfter;

	commandList->ResourceBarrier(1, &barrierDesc);
}

void DX12Utils::SetResourceTransitionBarriers(ID3D12GraphicsCommandList* commandList, std::vector<ID3D12Resource*> resources, std::vector<D3D12_RESOURCE_STATES> statesBefore, std::vector<D3D12_RESOURCE_STATES> statesAfter, std::vector<UINT> subResources) {
	auto numBarriers = resources.size();
	std::vector<D3D12_RESOURCE_BARRIER> barrierDescs(numBarriers);

	for (auto i = 0; i < numBarriers; i++) {
		barrierDescs[i].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrierDescs[i].Transition.pResource = resources[i];
		barrierDescs[i].Transition.Subresource = subResources[i];
		barrierDescs[i].Transition.StateBefore = statesBefore[i];
		barrierDescs[i].Transition.StateAfter = statesAfter[i];
	}

	commandList->ResourceBarrier(barrierDescs.size(), barrierDescs.data());
}

void DX12Utils::SetResourceTransitionBarriers(ID3D12GraphicsCommandList* commandList, std::vector<DX12ATexture*> textures, std::vector<D3D12_RESOURCE_STATES> statesAfter) {
	auto numBarriers = textures.size();
	std::vector<D3D12_RESOURCE_BARRIER> barrierDescs;
	barrierDescs.reserve(numBarriers);

	for (auto i = 0; i < numBarriers; i++) {
		if (textures[i]->getState() == statesAfter[i]) continue;

		auto& desc = barrierDescs.emplace_back();
		desc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		desc.Transition.pResource = textures[i]->getResource();
		desc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		desc.Transition.StateBefore = textures[i]->getState();
		desc.Transition.StateAfter = statesAfter[i];

		textures[i]->setState(statesAfter[i]);
	}

	commandList->ResourceBarrier(barrierDescs.size(), barrierDescs.data());
}

void DX12Utils::SetResourceUAVBarrier(ID3D12GraphicsCommandList* commandList, ID3D12Resource* resource) {
	D3D12_RESOURCE_BARRIER barrierDesc = {};

	barrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	barrierDesc.UAV.pResource = resource;

	commandList->ResourceBarrier(1, &barrierDesc);
}

unsigned int DX12Utils::Align(unsigned int location, unsigned int alignment) {
	if ((0 == alignment) || (alignment & (alignment - 1))) {
		Logger::Error("Non-pow2 alignment");
	}

	return ((location + (alignment - 1)) & ~(alignment - 1));
}


DX12Utils::CPUSharedBuffer::CPUSharedBuffer(ID3D12Device* device, unsigned int byteSize, D3D12_RESOURCE_FLAGS flags, D3D12_RESOURCE_STATES initState, D3D12_HEAP_PROPERTIES heapProps) 
	: m_bufferBegin(nullptr)
	, m_bufferCurrent(nullptr)
	, m_bufferEnd(nullptr)
{
	m_buffer = CreateBuffer(device, byteSize, flags, initState, heapProps);
	m_buffer->SetName(L"CPUSharedBuffer");

	assert(heapProps.Type != D3D12_HEAP_TYPE_DEFAULT && "Default heap type not supported as CPUSharedBuffer (for obvious reasons)");

	// Keep the buffer mapped at all times
	void* pData;
	D3D12_RANGE readRange{ 0, 0 };
	m_buffer->Map(0, &readRange, &pData);
	m_bufferCurrent = m_bufferBegin = reinterpret_cast<UINT8*>(pData);
	m_bufferEnd = m_bufferBegin + byteSize;
}

DX12Utils::CPUSharedBuffer::~CPUSharedBuffer() {
	m_buffer->Unmap(0, nullptr);
	m_buffer->Release();
}

D3D12_GPU_VIRTUAL_ADDRESS DX12Utils::CPUSharedBuffer::suballocate(unsigned int byteSize, unsigned int byteAlignment, void** outMappedBuffer) {
	unsigned int padding = (byteAlignment - (byteSize % byteAlignment)) % byteAlignment;
	m_bufferCurrent += padding;

	*outMappedBuffer = m_bufferCurrent;

	unsigned int offset = m_bufferCurrent - m_bufferBegin;
	m_bufferCurrent += byteSize;

	assert(m_bufferCurrent <= m_bufferEnd && "Tried to suballocate more memory than what is available");

	return m_buffer->GetGPUVirtualAddress() + offset;
}

D3D12_GPU_VIRTUAL_ADDRESS DX12Utils::CPUSharedBuffer::setData(void* data, unsigned int dataByteSize, unsigned int alignment) {
	void* mappedBuffer = nullptr;
	auto addr = suballocate(dataByteSize, alignment, &mappedBuffer);
	memcpy(mappedBuffer, data, dataByteSize);
	return addr;
}

void DX12Utils::CPUSharedBuffer::setCurrentPointerOffset(unsigned int offset) {
	m_bufferCurrent = m_bufferBegin + offset;
}

ID3D12Resource* DX12Utils::CPUSharedBuffer::getResource() {
	return m_buffer;
}


DX12Utils::GPUOnlyBuffer::GPUOnlyBuffer(ID3D12Device* device, unsigned int byteSize, D3D12_RESOURCE_FLAGS flags, D3D12_RESOURCE_STATES initState, D3D12_HEAP_PROPERTIES heapProps)
	: m_currentOffset(0)
	, m_bufferSize(byteSize)
{
	m_buffer = CreateBuffer(device, byteSize, flags, initState, heapProps);
	m_buffer->SetName(L"GPUOnlyBuffer");

	assert(heapProps.Type == D3D12_HEAP_TYPE_DEFAULT && "Only default heap type is supported as GPUOnlyBuffer");
}

DX12Utils::GPUOnlyBuffer::~GPUOnlyBuffer() {
	m_buffer->Release();
}

D3D12_GPU_VIRTUAL_ADDRESS DX12Utils::GPUOnlyBuffer::suballocate(unsigned int byteSize, unsigned int byteAlignment) {
	m_currentOffset = Align(m_currentOffset, byteAlignment);

	unsigned int offset = m_currentOffset;
	m_currentOffset += byteSize;

	assert(m_currentOffset <= m_bufferSize && "Tried to suballocate more memory than what is available");

	return m_buffer->GetGPUVirtualAddress() + offset;
}

void DX12Utils::GPUOnlyBuffer::setCurrentPointerOffset(unsigned int offset) {
	m_currentOffset = offset;
}

ID3D12Resource* DX12Utils::GPUOnlyBuffer::getResource() {
	return m_buffer;
}


// RootSignatureBuilder
D3D12_STATIC_SAMPLER_DESC DX12Utils::RootSignature::sDefaultSampler = { D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP, 0.f, 1, D3D12_COMPARISON_FUNC_ALWAYS, D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE, 0.f, FLT_MAX, 0, 0, D3D12_SHADER_VISIBILITY_ALL };

DX12Utils::RootSignature::RootSignature(const std::string& name)
	: m_name(name)
{ }

void DX12Utils::RootSignature::add32BitConstants() {
	assert(false && "Not implemented");
}

void DX12Utils::RootSignature::addDescriptorTable(const std::string& name, D3D12_DESCRIPTOR_RANGE_TYPE type, unsigned int shaderRegister, unsigned int space, unsigned int numDescriptors) {
	m_order.emplace_back(name);

	D3D12_DESCRIPTOR_RANGE* range = SAIL_NEW D3D12_DESCRIPTOR_RANGE;
	range->BaseShaderRegister = shaderRegister;
	range->RegisterSpace = space;
	range->NumDescriptors = numDescriptors;
	range->RangeType = type;
	range->OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER rootParam = {};
	rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParam.DescriptorTable.NumDescriptorRanges = 1;
	rootParam.DescriptorTable.pDescriptorRanges = range;

	m_rootParams.push_back(rootParam);
}

void DX12Utils::RootSignature::addCBV(const std::string& name, unsigned int shaderRegister, unsigned int space) {
	m_order.emplace_back(name);

	D3D12_ROOT_PARAMETER rootParam = {};
	rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParam.Descriptor.ShaderRegister = shaderRegister;
	rootParam.Descriptor.RegisterSpace = space;

	m_rootParams.push_back(rootParam);
}

void DX12Utils::RootSignature::addSRV(const std::string& name, unsigned int shaderRegister, unsigned int space) {
	m_order.emplace_back(name);

	D3D12_ROOT_PARAMETER rootParam = {};
	rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	rootParam.Descriptor.ShaderRegister = shaderRegister;
	rootParam.Descriptor.RegisterSpace = space;

	m_rootParams.push_back(rootParam);
}

void DX12Utils::RootSignature::addUAV(const std::string& name, unsigned int shaderRegister, unsigned int space) {
	m_order.emplace_back(name);

	D3D12_ROOT_PARAMETER rootParam = {};
	rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
	rootParam.Descriptor.ShaderRegister = shaderRegister;
	rootParam.Descriptor.RegisterSpace = space;

	m_rootParams.push_back(rootParam);
}

void DX12Utils::RootSignature::addStaticSampler(const D3D12_STATIC_SAMPLER_DESC& desc /*= sDefaultSampler*/) {
	m_staticSamplerDescs.push_back(desc);
}

void DX12Utils::RootSignature::build(ID3D12Device5* device, const D3D12_ROOT_SIGNATURE_FLAGS& flags) {
	D3D12_ROOT_SIGNATURE_DESC desc = {};
	desc.NumParameters = (UINT)m_rootParams.size();
	desc.pParameters = m_rootParams.data();
	desc.Flags = flags;
	desc.NumStaticSamplers = (UINT)m_staticSamplerDescs.size();
	desc.pStaticSamplers = m_staticSamplerDescs.data();

	ID3DBlob* sigBlob;
	ID3DBlob* errorBlob;
	ThrowIfBlobError(D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, &sigBlob, &errorBlob), errorBlob);
	device->CreateRootSignature(0, sigBlob->GetBufferPointer(), sigBlob->GetBufferSize(), IID_PPV_ARGS(&m_signature));

	std::wstring name = std::wstring(m_name.begin(), m_name.end());
	m_signature->SetName(name.c_str());

	// Delete memory allocated by descriptor tables
	for (auto& param : m_rootParams) {
		if (param.ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE)
			delete param.DescriptorTable.pDescriptorRanges;
	}
	// Delete data that is not needed after build
	m_rootParams.clear();
	m_staticSamplerDescs.clear();
}

ID3D12RootSignature** DX12Utils::RootSignature::get() {
	return m_signature.GetAddressOf();
}

unsigned int DX12Utils::RootSignature::getIndex(const std::string& name) {
	return static_cast<unsigned int>(std::find(m_order.begin(), m_order.end(), name) - m_order.begin());
}

void DX12Utils::RootSignature::doInOrder(std::function<void(const std::string&)> func) {
	for (auto& it : m_order) {
		func(it);
	}
}

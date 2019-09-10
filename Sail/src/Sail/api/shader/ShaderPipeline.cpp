#include "pch.h"
#include "ShaderPipeline.h"
#include "Sail/Application.h"
#include <regex>

ShaderPipeline* ShaderPipeline::CurrentlyBoundShader = nullptr;
const std::string ShaderPipeline::DEFAULT_SHADER_LOCATION = "res/shaders/";

using namespace Utils::String;

ShaderPipeline::ShaderPipeline(const std::string& filename)
	: vsBlob(nullptr)
	, gsBlob(nullptr)
	, psBlob(nullptr)
	, dsBlob(nullptr)
	, hsBlob(nullptr)
	, filename(filename)
{
	inputLayout = std::unique_ptr<InputLayout>(InputLayout::Create());
}

ShaderPipeline::~ShaderPipeline() {
	//Memory::safeRelease(VSBlob); // Do this?
}

void ShaderPipeline::compile() {
	std::string filepath = DEFAULT_SHADER_LOCATION + filename;
	std::string source = Utils::readFile(filepath);
	if (source == "")
		Logger::Error("Shader file is empty or does not exist: " + filename);
	parse(source);

	if (parsedData.hasVS) {
		vsBlob = compileShader(source, filepath, ShaderComponent::VS);
		//Memory::safeRelease(VSBlob); // is this right?
	}
	if (parsedData.hasPS) {
		psBlob = compileShader(source, filepath, ShaderComponent::PS);
		//Memory::safeRelease(blob);
	}
	if (parsedData.hasGS) {
		gsBlob = compileShader(source, filepath, ShaderComponent::GS);
		//Memory::safeRelease(blob);
	}
	if (parsedData.hasDS) {
		dsBlob = compileShader(source, filepath, ShaderComponent::DS);
		//Memory::safeRelease(blob);
	}
	if (parsedData.hasHS) {
		hsBlob = compileShader(source, filepath, ShaderComponent::HS);
		//Memory::safeRelease(blob);
	}
}

void ShaderPipeline::finish() {
}

void ShaderPipeline::bind(void* cmdList) {
	// Don't bind if already bound
	// This is to cut down on shader state changes
	if (CurrentlyBoundShader == this)
		return;

	for (auto& it : parsedData.cBuffers) {
		it.cBuffer->bind(cmdList);
	}
	for (auto& it : parsedData.samplers) {
		it.sampler->bind();
	}



	// Set input layout as active
	inputLayout->bind();
}

void ShaderPipeline::parse(const std::string& source) {
	
	// Find what shader types are contained in the source
	if (source.find("VSMain") != std::string::npos) parsedData.hasVS = true;
	if (source.find("PSMain") != std::string::npos) parsedData.hasPS = true;
	if (source.find("GSMain") != std::string::npos) parsedData.hasGS = true;
	if (source.find("DSMain") != std::string::npos) parsedData.hasDS = true;
	if (source.find("HSMain") != std::string::npos) parsedData.hasHS = true;

	// Remove comments from source
	std::string cleanSource = removeComments(source);

	const char* src;

	// Count and reserve memory for the vector of parsed data
	// This is needed to avoid copying/destructor calling
	{
		int numCBuffers = 0;
		src = cleanSource.c_str();
		while (src = findToken("cbuffer", src))	numCBuffers++;
		parsedData.cBuffers.reserve(numCBuffers);
	}
	{
		int numSamplers = 0;
		src = cleanSource.c_str();
		while (src = findToken("SamplerState", src)) numSamplers++;
		parsedData.samplers.reserve(numSamplers);
	}
	
	// Process all CBuffers
	src = cleanSource.c_str();
	while (src = findToken("cbuffer", src)) {
		parseCBuffer(getBlockStartingFrom(src));
	}

	// Process all samplers
	src = cleanSource.c_str();
	while (src = findToken("SamplerState", src)) {
		parseSampler(src);
	}

	// Process all textures
	src = cleanSource.c_str();
	while (src = findToken("Texture2D", src)) {
		parseTexture(src);
	}

}

void ShaderPipeline::parseCBuffer(const std::string& source) {

	const char* src = source.c_str();

	std::string bufferName = nextToken(src);
	auto bindShader = getBindShaderFromName(bufferName);


	int registerSlot = findNextIntOnLine(src);
	src = findToken("{", src); // Place ptr on same line as starting bracket
	src = nextLine(src);

	//Logger::Log("Slot: " + std::to_string(registerSlot));

	UINT size = 0;
	std::vector<ShaderCBuffer::CBufferVariable> vars;

	while (src < source.c_str() + source.size()) {
		std::string type = nextToken(src);
		src += type.size();
		UINT tokenSize;
		std::string name = nextTokenAsName(src, tokenSize, true);
		src = nextLine(src);

		// Push the name and byteOffset to the vector
		vars.push_back({name, size});
		size += getSizeOfType(type);

		/*Logger::Log("Type: " + type);
		Logger::Log("Name: " + name);*/
	}

	// Memory align to 16 bytes
	if (size % 16 != 0)
		size = size - (size % 16) + 16;

	void* initData = malloc(size);
	memset(initData, 0, size);
	parsedData.cBuffers.emplace_back(vars, initData, size, bindShader, registerSlot);
	free(initData);

	//Logger::Log(src);
}

void ShaderPipeline::parseSampler(const char* source) {
	UINT tokenSize = 0;
	std::string name = nextTokenAsName(source, tokenSize);
	source += tokenSize;

	auto bindShader = getBindShaderFromName(name);

	int slot = findNextIntOnLine(source);
	if (slot == -1) slot = 0; // No slot specified, use 0 as default

	ShaderResource res(name, static_cast<UINT>(slot));
	parsedData.samplers.emplace_back(res, Texture::WRAP, Texture::MIN_MAG_MIP_LINEAR, bindShader, slot);
}

void ShaderPipeline::parseTexture(const char* source) {
	UINT tokenSize = 0;
	std::string name = nextTokenAsName(source, tokenSize);
	source += tokenSize;

	int slot = findNextIntOnLine(source);
	if (slot == -1) slot = 0; // No slot specified, use 0 as default

	parsedData.textures.emplace_back(name, slot);
}

std::string ShaderPipeline::nextTokenAsName(const char* source, UINT& outTokenSize, bool allowArray) const {
	std::string name = nextToken(source);
	outTokenSize = name.size();
	if (name[name.size() - 1] == ';')
		name = name.substr(0, name.size() - 1); // Remove ending ';'
	bool isArray = name[name.size() - 1] == ']';
	if (!allowArray && isArray)
		Logger::Error("Shader resource with name \"" + name + "\" is of unsupported type - array");
	if (isArray) {
		// remove [asd] part from the name
		auto start = name.find("[");
		auto size = name.substr(start).find(']') + 1;
		name.erase(start, size);
	}
	return name;
}

ShaderComponent::BIND_SHADER ShaderPipeline::getBindShaderFromName(const std::string& name) const {
	if (startsWith(name.c_str(), "VSPS") || startsWith(name.c_str(), "PSVS")) return ShaderComponent::BIND_SHADER(ShaderComponent::VS | ShaderComponent::PS);
	if (startsWith(name.c_str(), "VS")) return ShaderComponent::VS;
	if (startsWith(name.c_str(), "PS")) return ShaderComponent::PS;
	if (startsWith(name.c_str(), "GS")) return ShaderComponent::GS;
	if (startsWith(name.c_str(), "DS")) return ShaderComponent::DS;
	if (startsWith(name.c_str(), "HS")) return ShaderComponent::HS;
	if (startsWith(name.c_str(), "CS")) return ShaderComponent::CS;
	Logger::Warning("Shader resource with name \"" + name + "\" not starting with VS/PS etc, using VS as default in shader: \"" + filename + "\"");
	return ShaderComponent::VS; // Default to binding to VertexShader
}

//void ShaderPipeline::bind() {
//
//	// Don't bind if already bound
//	// This is to cut down on shader state changes
//	if (CurrentlyBoundShader == this)
//		return;
//
//	//auto* devCon = Application::getInstance()->getAPI()->getDeviceContext();
//
//	if (m_vs)	m_vs->bind();
//	//else		devCon->VSSetShader(nullptr, 0, 0);
//	if (m_gs)	m_gs->bind();
//	//else		devCon->GSSetShader(nullptr, 0, 0);
//	if (m_ps)	m_ps->bind();
//	//else		devCon->PSSetShader(nullptr, 0, 0);
//	if (m_ds)	m_ds->bind();
//	//else		devCon->DSSetShader(nullptr, 0, 0);
//	if (m_hs)	m_hs->bind();
//	//else		devCon->HSSetShader(nullptr, 0, 0);
//
//	for (auto& it : parsedData.cBuffers) {
//		it.cBuffer.bind();
//	}
//	for (auto& it : parsedData.samplers) {
//		it.sampler.bind();
//	}
//
//	// Set input layout as active
//	inputLayout->bind();
//
//	// Set this shader as bound
//	CurrentlyBoundShader = this;
//
//}
//
//void ShaderPipeline::bindCS(UINT csIndex) {
//	/*if (m_css[csIndex]) 
//		m_css[csIndex]->bind();*/
//}

//ID3D10Blob* ShaderPipeline::compileShader(const std::string& source, const std::string& entryPoint, const std::string& shaderVersion) {
//	
//	ID3D10Blob *shader = nullptr;
//	ID3D10Blob *errorMsg;
//	if (FAILED(D3DCompile(source.c_str(), source.size(), DEFAULT_SHADER_LOCATION.c_str(), NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint.c_str(), shaderVersion.c_str(), D3DCOMPILE_DEBUG, 0, &shader, &errorMsg))) {
//		OutputDebugString(L"\n Failed to compile shader\n\n");
//
//		char* msg = (char*)(errorMsg->GetBufferPointer());
//
//		std::stringstream ss;
//		ss << "Failed to compile shader (" << entryPoint << ", " << shaderVersion << ")\n";
//
//		for (size_t i = 0; i < errorMsg->GetBufferSize(); i++) {
//			ss << msg[i];
//		}
//		Logger::Error(ss.str());
//	}
//
//	return shader;
//
//}

void ShaderPipeline::setWireframe(bool wireframeState) {
	wireframe = wireframeState;

}

InputLayout& ShaderPipeline::getInputLayout() {
	return *inputLayout.get();
}

void* ShaderPipeline::getVsBlob() {
	return vsBlob;
}

const std::string& ShaderPipeline::getName() const {
	return filename;
}

// TODO: size isnt really needed, can be read from the byteOffset of the next var
void ShaderPipeline::setCBufferVar(const std::string& name, const void* data, UINT size) {
	bool success = trySetCBufferVar(name, data, size);
	if (!success)
		Logger::Warning("Tried to set CBuffer variable that did not exist (" + name + ")");
}

bool ShaderPipeline::trySetCBufferVar(const std::string& name, const void* data, UINT size) {
	for (auto& it : parsedData.cBuffers) {
		for (auto& var : it.vars) {
			if (var.name == name) {
				ShaderComponent::ConstantBuffer& cbuffer = *it.cBuffer.get();
				cbuffer.updateData(data, size, var.byteOffset);
				return true;
			}
		}
	}
	return false;
}

//void ShaderPipeline::setTexture2D(const std::string& name, ID3D11ShaderResourceView* srv) {
//
//	UINT slot = findSlotFromName(name, parsedData.textures);
//	//Application::getInstance()->getAPI<DX11API>()->getDeviceContext()->PSSetShaderResources(slot, 1, &srv);
//
//}

//void ShaderPipeline::setVertexShader(void* blob) {
//
//	//m_vs = std::make_unique<VertexShader>(blob);
//
//}
//void ShaderPipeline::setGeometryShader(void* blob) {
//
//	//m_gs = std::make_unique<GeometryShader>(blob);
//
//}
//void ShaderPipeline::setPixelShader(void* blob) {
//
//	//m_ps = std::make_unique<PixelShader>(blob);
//
//}
//void ShaderPipeline::setComputeShaders(void** blob, UINT numBlobs) {
//	/*m_css.resize(numBlobs);
//
//	for (UINT i = 0; i < numBlobs; i++) {
//		m_css[i] = std::make_unique<ComputeShader>(blob[i]);
//	}*/
//}
//void ShaderPipeline::setDomainShader(void* blob) {
//
//	//m_ds = std::make_unique<DomainShader>(blob);
//
//}
//void ShaderPipeline::setHullShader(void* blob) {
//
//	//m_hs = std::make_unique<HullShader>(blob);
//
//}

// TODO: registerTypeSize(typeName, size)
UINT ShaderPipeline::getSizeOfType(const std::string& typeName) const {
	if (typeName == "float") return 4;
	if (typeName == "float2") return 4*2;
	if (typeName == "float3") return 4*3;
	if (typeName == "float4") return 4*4;
	if (typeName == "float3x3") return 4 * 3 * 3;
	if (typeName == "float4x4" || typeName == "matrix") return 4 * 4 * 4;

	if (typeName == "Material") return 48;
	if (typeName == "DirectionalLight") return 32;
	if (typeName == "PointLight") return 32;
	if (typeName == "PointLightInput") return 272;
	if (typeName == "DeferredPointLightData") return 48;
	if (typeName == "DeferredDirLightData") return 32;
	//if (typeName == "DeferredPointLightData") return 48;
	//if (typeName == "PointLightInput") return 384;


	Logger::Error("Found shader variable type with unknown size (" + typeName + ")");
	return 0;
}

UINT ShaderPipeline::findSlotFromName(const std::string& name, const std::vector<ShaderResource>& resources) const {
	for (auto& resource : resources) {
		if (resource.name == name)
			return resource.slot;
	}
	Logger::Error("Could not find shader resource named \"" + name + "\"");
	return -1;
}
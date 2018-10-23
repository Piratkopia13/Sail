#include "ShaderSet.h"
#include "../../api/Application.h"
#include <regex>

ShaderSet* ShaderSet::CurrentlyBoundShader = nullptr;

using namespace Utils::String;

ShaderSet::ShaderSet(const std::string& filename)
	: m_vs(nullptr)
	, m_gs(nullptr)
	, m_ps(nullptr)
	, m_ds(nullptr)
	, m_hs(nullptr)
	, VSBlob(nullptr)
	, filename(filename)
{
	std::string source = Utils::readFile(DEFAULT_SHADER_LOCATION + filename);
	if (source == "")
		Logger::Error("Shader file is empty or does not exist: " + filename);
	parse(source);

	if (m_parsedData.hasVS) {
		VSBlob = compileShader(source, "VSMain", "vs_5_0");
		setVertexShader(VSBlob);
	}
	if (m_parsedData.hasPS) {
		ID3D10Blob* blob = compileShader(source, "PSMain", "ps_5_0");
		setPixelShader(blob);
		Memory::safeRelease(blob);
	}
	if (m_parsedData.hasGS) {
		ID3D10Blob* blob = compileShader(source, "GSMain", "gs_5_0");
		setGeometryShader(blob);
		Memory::safeRelease(blob);
	}
	if (m_parsedData.hasDS) {
		ID3D10Blob* blob = compileShader(source, "DSMain", "ds_5_0");
		setDomainShader(blob);
		Memory::safeRelease(blob);
	}
	if (m_parsedData.hasHS) {
		ID3D10Blob* blob = compileShader(source, "HSMain", "hs_5_0");
		setHullShader(blob);
		Memory::safeRelease(blob);
	}
}

ShaderSet::~ShaderSet() {
	Memory::safeRelease(VSBlob);
}

void ShaderSet::parse(const std::string& source) {
	
	// Find what shader types are contained in the source
	if (source.find("VSMain") != std::string::npos) m_parsedData.hasVS = true;
	if (source.find("PSMain") != std::string::npos) m_parsedData.hasPS = true;
	if (source.find("GSMain") != std::string::npos) m_parsedData.hasGS = true;
	if (source.find("DSMain") != std::string::npos) m_parsedData.hasDS = true;
	if (source.find("HSMain") != std::string::npos) m_parsedData.hasHS = true;

	// TODO: remove comments from source
	std::string cleanSource = removeComments(source);

	const char* src;

	// Count and reserve memory for the vector of parsed data
	// This is needed to avoid copying/destructor calling
	{
		int numCBuffers = 0;
		src = cleanSource.c_str();
		while (src = findToken("cbuffer", src))	numCBuffers++;
		m_parsedData.cBuffers.reserve(numCBuffers);
	}
	{
		int numSamplers = 0;
		src = cleanSource.c_str();
		while (src = findToken("SamplerState", src)) numSamplers++;
		m_parsedData.samplers.reserve(numSamplers);
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

void ShaderSet::parseCBuffer(const std::string& source) {

	const char* src = source.c_str();

	std::string bufferName = nextToken(src);
	auto bindShader = getBindShaderFromName(bufferName);


	int registerSlot = findNextIntOnLine(src);
	src = findToken("{", src); // Place ptr on same line as starting bracket
	src = nextLine(src);

	Logger::Log("Slot: " + std::to_string(registerSlot));

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

		Logger::Log("Type: " + type);
		Logger::Log("Name: " + name);
	}

	// Memory align to 16 bytes
	if (size % 16 != 0)
		size = size - (size % 16) + 16;

	void* initData = malloc(size);
	memset(initData, 0, size);
	m_parsedData.cBuffers.emplace_back(vars, initData, size, bindShader, registerSlot);
	free(initData);

	Logger::Log(src);
}

void ShaderSet::parseSampler(const char* source) {
	UINT tokenSize = 0;
	std::string name = nextTokenAsName(source, tokenSize);
	source += tokenSize;

	auto bindShader = getBindShaderFromName(name);

	int slot = findNextIntOnLine(source);
	if (slot == -1) slot = 0; // No slot specified, use 0 as default

	ShaderResource res(name, static_cast<UINT>(slot));
	m_parsedData.samplers.emplace_back(res, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_FILTER_MIN_MAG_MIP_POINT, bindShader, slot);
}

void ShaderSet::parseTexture(const char* source) {
	UINT tokenSize = 0;
	std::string name = nextTokenAsName(source, tokenSize);
	source += tokenSize;

	int slot = findNextIntOnLine(source);
	if (slot == -1) slot = 0; // No slot specified, use 0 as default

	m_parsedData.textures.emplace_back(name, slot);
}

std::string ShaderSet::nextTokenAsName(const char* source, UINT& outTokenSize, bool allowArray) const {
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

ShaderComponent::BIND_SHADER ShaderSet::getBindShaderFromName(const std::string& name) const {
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

void ShaderSet::bind() {

	// Don't bind if already bound
	// This is to cut down on shader state changes
	/*if (CurrentlyBoundShader == this)
		return;*/

	auto* devCon = Application::getInstance()->getAPI()->getDeviceContext();

	if (m_vs)	m_vs->bind();
	else		devCon->VSSetShader(nullptr, 0, 0);
	if (m_gs)	m_gs->bind();
	else		devCon->GSSetShader(nullptr, 0, 0);
	if (m_ps)	m_ps->bind();
	else		devCon->PSSetShader(nullptr, 0, 0);
	if (m_ds)	m_ds->bind();
	else		devCon->DSSetShader(nullptr, 0, 0);
	if (m_hs)	m_hs->bind();
	else		devCon->HSSetShader(nullptr, 0, 0);

	for (auto& it : m_parsedData.cBuffers) {
		it.cBuffer.bind();
	}
	for (auto& it : m_parsedData.samplers) {
		it.sampler.bind();
	}

	// Set input layout as active
	inputLayout.bind();

	// Set this shader as bound
	CurrentlyBoundShader = this;

}

void ShaderSet::bindCS(UINT csIndex) {
	if (m_css[csIndex]) 
		m_css[csIndex]->bind();
}

ID3D10Blob* ShaderSet::compileShader(const std::string& source, const std::string& entryPoint, const std::string& shaderVersion) {
	
	ID3D10Blob *shader = nullptr;
	ID3D10Blob *errorMsg;
	if (FAILED(D3DCompile(source.c_str(), source.size(), DEFAULT_SHADER_LOCATION.c_str(), NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint.c_str(), shaderVersion.c_str(), D3DCOMPILE_DEBUG, 0, &shader, &errorMsg))) {
		OutputDebugString(L"\n Failed to compile shader\n\n");

		char* msg = (char*)(errorMsg->GetBufferPointer());

		std::stringstream ss;
		ss << "Failed to compile shader (" << entryPoint << ", " << shaderVersion << ")\n";

		for (size_t i = 0; i < errorMsg->GetBufferSize(); i++) {
			ss << msg[i];
		}
		Logger::Error(ss.str());
	}

	return shader;

}

const InputLayout& ShaderSet::getInputLayout() const {
	return inputLayout;
}

// TODO: size isnt really needed, can be read from the byteOffset of the next var
void ShaderSet::setCBufferVar(const std::string& name, const void* data, UINT size) {
	bool success = trySetCBufferVar(name, data, size);
	if (!success)
		Logger::Warning("Tried to set CBuffer variable that did not exist (" + name + ")");
}

bool ShaderSet::trySetCBufferVar(const std::string& name, const void* data, UINT size) {
	for (auto& it : m_parsedData.cBuffers) {
		for (auto& var : it.vars) {
			if (var.name == name) {
				ShaderComponent::ConstantBuffer& cbuffer = it.cBuffer;
				cbuffer.updateData(data, size, var.byteOffset);
				return true;
			}
		}
	}
	return false;
}

void ShaderSet::setTexture2D(const std::string& name, ID3D11ShaderResourceView* srv) {

	UINT slot = findSlotFromName(name, m_parsedData.textures);
	Application::getInstance()->getAPI()->getDeviceContext()->PSSetShaderResources(slot, 1, &srv);

}

void ShaderSet::setVertexShader(ID3D10Blob* blob) {

	m_vs = std::make_unique<VertexShader>(blob);

}
void ShaderSet::setGeometryShader(ID3D10Blob* blob) {

	m_gs = std::make_unique<GeometryShader>(blob);

}
void ShaderSet::setPixelShader(ID3D10Blob* blob) {

	m_ps = std::make_unique<PixelShader>(blob);

}
void ShaderSet::setComputeShaders(ID3D10Blob** blob, UINT numBlobs) {
	m_css.resize(numBlobs);

	for (UINT i = 0; i < numBlobs; i++) {
		m_css[i] = std::make_unique<ComputeShader>(blob[i]);
	}
}
void ShaderSet::setDomainShader(ID3D10Blob* blob) {

	m_ds = std::make_unique<DomainShader>(blob);

}
void ShaderSet::setHullShader(ID3D10Blob* blob) {

	m_hs = std::make_unique<HullShader>(blob);

}

// TODO: registerTypeSize(typeName, size)
UINT ShaderSet::getSizeOfType(const std::string& typeName) const {
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

UINT ShaderSet::findSlotFromName(const std::string& name, const std::vector<ShaderResource>& resources) const {
	for (auto& resource : resources) {
		if (resource.name == name)
			return resource.slot;
	}
	Logger::Error("Could not find shader resource named \"" + name + "\"");
	return -1;
}
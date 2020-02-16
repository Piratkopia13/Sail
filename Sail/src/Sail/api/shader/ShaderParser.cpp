#include "pch.h"
#include "ShaderParser.h"

using namespace Utils::String;

ShaderParser::ShaderParser(const std::string& filename)
	: m_filename(filename)
{ }

const ShaderParser::ParsedData& ShaderParser::getParsedData() const {
	return m_parsedData;
}

void ShaderParser::parse(const std::string& source) {
	SAIL_PROFILE_FUNCTION();

	// Find what shader types are contained in the source
	if (source.find("VSMain") != std::string::npos) m_parsedData.hasVS = true;
	if (source.find("PSMain") != std::string::npos) m_parsedData.hasPS = true;
	if (source.find("GSMain") != std::string::npos) m_parsedData.hasGS = true;
	if (source.find("DSMain") != std::string::npos) m_parsedData.hasDS = true;
	if (source.find("HSMain") != std::string::npos) m_parsedData.hasHS = true;
	if (source.find("CSMain") != std::string::npos) m_parsedData.hasCS = true;

	if (!m_parsedData.hasVS && !m_parsedData.hasPS && !m_parsedData.hasGS && !m_parsedData.hasDS && !m_parsedData.hasHS && !m_parsedData.hasCS) {
		Logger::Error("No main function found in shader. The main function(s) needs to be named VSMain, PSMain, GSMain, DSMain, HSMain or CSMain");
		assert(false);
	}
	if (m_parsedData.hasCS && (m_parsedData.hasVS || m_parsedData.hasPS || m_parsedData.hasGS || m_parsedData.hasDS || m_parsedData.hasHS)) {
		Logger::Error("No other shader type (PS, VS, GS, HS, DS) is allowed in the same file as a compute shader!");
		assert(false);
	}

	// Remove comments from source
	std::string cleanSource = removeComments(source);

	const char* src;

	// Store used vertex data attributes and their order
	{
		src = cleanSource.c_str();
		if (m_parsedData.hasVS) {
			if (src = findToken("VSIn", src)) {
				unsigned int mul = 1;
				while (src && nextToken(src) != "};") {
					src = findToken(":", src);
					std::string semantic = nextToken(src);
					semantic = semantic.substr(0, semantic.length() - 1); // Remove trailing semicolon
					src = nextLine(src);
					
					if (semantic == "POSITION" || semantic == "POSITION0") {
						m_parsedData.attributesHash += InputLayout::POSITION * mul;;
						mul *= 10;
					}
					if (semantic == "TEXCOORD" || semantic == "TEXCOORD0") {
						m_parsedData.attributesHash += InputLayout::TEXCOORD * mul;;
						mul *= 10;
					}
					if (semantic == "NORMAL" || semantic == "NORMAL0") {
						m_parsedData.attributesHash += InputLayout::NORMAL * mul;;
						mul *= 10;
					}
					if (semantic == "TANGENT" || semantic == "TANGENT0") {
						m_parsedData.attributesHash += InputLayout::TANGENT * mul;;
						mul *= 10;
					}
					if (semantic == "BINORMAL" || semantic == "BINORMAL0") {
						m_parsedData.attributesHash += InputLayout::BITANGENT * mul;;
						mul *= 10;
					}

				}
			} else {
				Logger::Error("Found Vertex Shader with missing or incorrectly named input struct. The input has to be defined in a struct named \"VSIn\" \nIn source (" + m_filename + ")");
			}
		}
	}

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
	// RWTextures needs to be handled first!
	src = cleanSource.c_str();
	while (src = findToken("RWTexture2D", src)) {
		parseRWTexture(src);
	}
	src = cleanSource.c_str();
	while (src = findToken("Texture2D", src)) {
		parseTexture(src);
	}
	src = cleanSource.c_str();
	while (src = findToken("TextureCube", src)) {
		parseTexture(src);
	}

}

void ShaderParser::parseCBuffer(const std::string& source) {
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
		vars.push_back({ name, size });
		size += getSizeOfType(type);

		/*Logger::Log("Type: " + type);
		Logger::Log("Name: " + name);*/
	}

	// Memory align to 16 bytes
	if (size % 16 != 0)
		size = size - (size % 16) + 16;

	void* initData = malloc(size);
	memset(initData, 0, size);
	m_parsedData.cBuffers.emplace_back(vars, initData, size, bindShader, registerSlot, m_parsedData.hasCS);
	free(initData);

	//Logger::Log(src);
}

void ShaderParser::parseSampler(const char* source) {
	UINT tokenSize = 0;
	std::string name = nextTokenAsName(source, tokenSize);
	source += tokenSize;

	auto bindShader = getBindShaderFromName(name);

	int slot = findNextIntOnLine(source);
	if (slot == -1) slot = 0; // No slot specified, use 0 as default

	ShaderResource res(name, static_cast<UINT>(slot));
	m_parsedData.samplers.emplace_back(res, Texture::WRAP, Texture::ANISOTROPIC, bindShader, slot);
}

void ShaderParser::parseTexture(const char* source) {
	if (source[0] == '<') {
		// A type was found in the place of a name
		// This probably means that it is a RWTexture2D and has already been handled
		source = nextLine(source);
		return;
	}

	UINT tokenSize = 0;
	std::string name = nextTokenAsName(source, tokenSize);
	source += tokenSize;

	int slot = findNextIntOnLine(source);
	if (slot == -1) slot = 0; // No slot specified, use 0 as default

	m_parsedData.textures.emplace_back(name, slot);
}

void ShaderParser::parseRWTexture(const char* source) {
	UINT tokenSize = 0;
	std::string type = nextTokenAsType(source, tokenSize);
	source += tokenSize;

	tokenSize = 0;
	std::string name = nextTokenAsName(source, tokenSize);
	source += tokenSize;

	tokenSize = 0;
	int slot = findNextIntOnLine(source);
	if (slot == -1) {
		slot = 0; // No slot specified, use 0 as default
	}

	// Get texture format from source, if specified
	ResourceFormat::TextureFormat format = ResourceFormat::R8G8B8A8;
	const char* newLine = strchr(source, '\n');
	size_t lineLength = newLine - source;
	char* lineCopy = (char*)malloc(lineLength + 1);
	memset(lineCopy, '\0', lineLength + 1);
	strncpy_s(lineCopy, lineLength + 1, source, lineLength);
	if (strstr(lineCopy, "SAIL_RGBA16_FLOAT")) {
		format = ResourceFormat::R16G16B16A16_FLOAT;
	}
	free(lineCopy);

	std::string nameSuffix(" File: " + m_filename + " slot " + std::to_string(slot));
	m_parsedData.renderableTextures.emplace_back(ShaderResource(name, slot), format, nameSuffix);
}

std::string ShaderParser::nextTokenAsName(const char* source, UINT& outTokenSize, bool allowArray) const {
	std::string name = nextToken(source);
	outTokenSize = (UINT)name.size() + 1U; /// +1 to account for the space before the name
	if (name[name.size() - 1] == ';') {
		name = name.substr(0, name.size() - 1); // Remove ending ';'
	}
	bool isArray = name[name.size() - 1] == ']';
	if (!allowArray && isArray) {
		Logger::Error("Shader resource with name \"" + name + "\" is of unsupported type - array");
	}
	if (isArray) {
		// remove [asd] part from the name
		auto start = name.find("[");
		auto size = name.substr(start).find(']') + 1;
		name.erase(start, size);
	}
	return name;
}

std::string ShaderParser::nextTokenAsType(const char* source, UINT& outTokenSize) const {
	std::string type = nextToken(source);
	outTokenSize = (UINT)type.size();
	// Remove first '<' and last '>' character
	type = type.substr(1, type.size() - 2);

	return type;
}

ShaderComponent::BIND_SHADER ShaderParser::getBindShaderFromName(const std::string& name) const {
	if (startsWith(name.c_str(), "VSPS") || startsWith(name.c_str(), "PSVS")) return ShaderComponent::BIND_SHADER(ShaderComponent::VS | ShaderComponent::PS);
	if (startsWith(name.c_str(), "VS")) return ShaderComponent::VS;
	if (startsWith(name.c_str(), "PS")) return ShaderComponent::PS;
	if (startsWith(name.c_str(), "GS")) return ShaderComponent::GS;
	if (startsWith(name.c_str(), "DS")) return ShaderComponent::DS;
	if (startsWith(name.c_str(), "HS")) return ShaderComponent::HS;
	if (startsWith(name.c_str(), "CS")) return ShaderComponent::CS;
	Logger::Warning("Shader resource with name \"" + name + "\" not starting with VS/PS etc, using VS as default in shader: \"" + m_filename + "\"");
	return ShaderComponent::VS; // Default to binding to VertexShader
}

// TODO: registerTypeSize(typeName, size)
UINT ShaderParser::getSizeOfType(const std::string& typeName) const {
	if (typeName == "uint")								return 4;
	if (typeName == "bool")								return 4;
	if (typeName == "float")							return 4;
	if (typeName == "float2" ||
		typeName == "int2" ||
		typeName == "uint2")							return 4 * 2;
	if (typeName == "float3")							return 4 * 3;
	if (typeName == "float4")							return 4 * 4;
	if (typeName == "float3x3")							return 4 * 3 * 3;
	if (typeName == "float4x4" ||
		typeName == "matrix")							return 4 * 4 * 4;

	if (typeName == "PhongMaterial")					return 48;
	if (typeName == "PBRMaterial")						return 4 * 12;
	if (typeName == "DirectionalLight")					return 32;
	if (typeName == "PointLight")						return 32;
	if (typeName == "PointLightInput")					return 4 * 8 * 8; // last 8 is NUM_POINT_LIGHTS
	if (typeName == "DeferredPointLightData")			return 48;
	if (typeName == "DeferredDirLightData")				return 32;

	Logger::Error("Found shader variable type with unknown size (" + typeName + ")");
	return 0;
}

int ShaderParser::findSlotFromName(const std::string& name, const std::vector<ShaderResource>& resources) const {
	for (auto& resource : resources) {
		if (resource.name == name)
			return resource.slot;
	}
	//Logger::Error("Could not find shader resource named \"" + name + "\"");
	return -1;
}
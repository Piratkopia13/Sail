#include "pch.h"
#include "ShaderParser.h"
#include "Sampler.h"

using namespace Utils::String;

ShaderParser::ShaderParser(const std::string& filename)
	: m_filename(filename)
{ }

const ShaderParser::ParsedData& ShaderParser::getParsedData() const {
	return m_parsedData;
}

std::string ShaderParser::parse(const std::string& source) {
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
	m_cleanSource = removeComments(source);

	const char* src;

	// Store used vertex data attributes and their order
	{
		src = m_cleanSource.c_str();
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
		src = m_cleanSource.c_str();
		while (src = findToken("cbuffer", src))	numCBuffers++;
		m_parsedData.cBuffers.reserve(numCBuffers);
	}
	{
		int numSamplers = 0;
		src = m_cleanSource.c_str();
		while (src = findToken("SamplerState", src)) numSamplers++;
		m_parsedData.samplers.reserve(numSamplers);
	}

	// Process all CBuffers
	src = m_cleanSource.c_str();
	while (src = findToken("cbuffer", src)) {
		parseCBuffer(getBlockStartingFrom(src));
	}

	// Process all push constants
	src = m_cleanSource.c_str();
	while (src = findToken("vk::push_constant", src)) {
		std::string pushTokenSource = getBlockStartingFrom(src);
		src += pushTokenSource.size();
		const char* end = Utils::String::findToken(";", src);
		pushTokenSource += std::string(src, end-src);
		parseCBuffer(pushTokenSource, true);
	}

	// Process all samplers
	src = m_cleanSource.c_str();
	while (src = findToken("SamplerState", src)) {
		parseSampler(src, m_cleanSource);
	}

	// Process all textures
	// RWTextures needs to be handled first!
	src = m_cleanSource.c_str();
	while (src = findToken("RWTexture2D", src)) {
		parseRWTexture(src);
	}
	src = m_cleanSource.c_str();
	while (src = findToken("Texture2D", src)) {
		parseTexture(src);
	}
	src = m_cleanSource.c_str();
	while (src = findToken("TextureCube", src)) {
		parseTexture(src);
	}

	return m_cleanSource;
}

void ShaderParser::parseCBuffer(const std::string& source, bool storeAsPushConstant) {
	const char* src = source.c_str();
	const char* end = (storeAsPushConstant) ? findToken("}", src) - 1 : source.c_str() + source.size();

	std::string bufferName = "Unspecified";
	if (storeAsPushConstant) {
		// Name is stored right after the block
		const char* start = findToken("}", src);
		UINT size;
		bufferName = nextTokenAsName(start, size);
	} else {
		// Name is stored in the next token
		bufferName = nextToken(src);
	}
	auto bindShader = getBindShaderFromName(bufferName);


	int registerSlot = findNextIntOnLine(src);
	src = findToken("{", src); // Place ptr on same line as starting bracket
	src = nextLine(src);

	//Logger::Log("Slot: " + std::to_string(registerSlot));

	UINT size = 0;
	std::vector<ShaderCBuffer::CBufferVariable> vars;

	while (src < end) {
		std::string type = nextToken(src);
		src += type.size();
		UINT tokenSize;
		int elementsInArray = 1;
		std::string name = nextTokenAsName(src, tokenSize, &elementsInArray);
		src = nextLine(src);

		// Push the name and byteOffset to the vector
		vars.push_back({ name, size });
		size += getSizeOfType(type) * elementsInArray;

		/*Logger::Log("Type: " + type);
		Logger::Log("Name: " + name);*/
	}

	// Memory align to 16 bytes
	if (size % 16 != 0)
		size = size - (size % 16) + 16;

	if (storeAsPushConstant) {
		m_parsedData.pushConstants.emplace_back(vars, size, bindShader);
	} else {
		void* initData = malloc(size);
		memset(initData, 0, size);
		m_parsedData.cBuffers.emplace_back(vars, initData, size, bindShader, registerSlot, m_parsedData.hasCS);
		free(initData);
	}

	//Logger::Log(src);
}

void ShaderParser::parseSampler(const char* sourceChar, std::string& source) {
	UINT tokenSize = 0;
	std::string name = nextTokenAsName(sourceChar, tokenSize);
	sourceChar += tokenSize;

	const char* newLine = strchr(sourceChar, '\n');
	size_t lineLength = newLine - sourceChar;
	char* lineCopy = (char*)malloc(lineLength + 1);
	memset(lineCopy, '\0', lineLength + 1);
	strncpy_s(lineCopy, lineLength + 1, sourceChar, lineLength);

	// Parse "SAIL_X" macros in source and replace register slot if found
	bool replaceRegisterSlot = false;
	ShaderComponent::Sampler::ShaderInfo samplerInfo;
	auto& samplerMap = ShaderComponent::Sampler::GetShaderSlotsMap();
	std::string macro;
	for (auto& it : samplerMap) {
		if (strstr(lineCopy, it.first.c_str())) {
			samplerInfo = it.second;
			replaceRegisterSlot = true;
			macro = it.first;
			break;
		}
	}
	assert(!(!replaceRegisterSlot && strstr(lineCopy, "SAIL_")) && "Unrecognized macro found on sampler");
	free(lineCopy);
	
	int slot = 0;
	if (replaceRegisterSlot) {
		slot = samplerInfo.slot;
		std::string newReg = "register(s";
		newReg += std::to_string(slot);
		newReg += "); // SLOT REPLACED BY SAIL SHADERPARSER!";
		source.replace(source.find(macro), macro.length()+1, newReg); // +1 to include the ending semicolon
	} else {
		slot = findNextIntOnLine(sourceChar);
		if (slot == -1) slot = 0; // No slot specified, use 0 as default
	}

	auto bindShader = getBindShaderFromName(name);
	auto uslot = static_cast<unsigned int>(slot);
	ShaderResource res(name, uslot, 1, uslot);
	m_parsedData.samplers.emplace_back(res, samplerInfo.addressMode, samplerInfo.filter, bindShader, slot);
}

void ShaderParser::parseTexture(const char* source) {
	const char* lineStart = source;
	while (true) {
		if (lineStart == m_cleanSource.c_str() || // Out of bounds check
			lineStart[-1] == '\n') { // if the "next" character is a new line, we have found the line start
			break;
		}
		lineStart--;
	}

	if (source[0] == '<') {
		// A type was found in the place of a name
		// This probably means that it is a RWTexture2D and has already been handled
		source = nextLine(source);
		return;
	}

	UINT tokenSize = 0;
	int arrSize = 1;
	std::string name = nextTokenAsName(source, tokenSize, &arrSize);
	source += tokenSize;

	int slot = findNextIntOnLine(source);
	if (slot == -1) slot = 0; // No slot specified, use 0 as default

	int vkBinding = slot;
	// if vk::binding is used, use that as the vk slot. 
	// vk::binding is allowed on the previous line or at the start of the current line.
	auto matchOnCurrentLine = findToken("vk::binding", lineStart, true);
	if (matchOnCurrentLine) {
		vkBinding = findNextIntOnLine(matchOnCurrentLine);
	} else {
		int newLines = 0;
		while (newLines < 2) {
			if (lineStart == m_cleanSource.c_str()) break; // Out of bounds check
			if (lineStart[0] == '\n') newLines++;
			lineStart--;
		}
		lineStart+=2;
		auto matchOnPrevLine = findToken("vk::binding", lineStart, true);
		if (matchOnPrevLine) {
			vkBinding = findNextIntOnLine(matchOnPrevLine);
		}
	}


	m_parsedData.textures.emplace_back(name, slot, arrSize, vkBinding);
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

	// Store name and slot as a texture to allow shader to manually bind this slot
	auto uslot = static_cast<unsigned int>(slot);
	m_parsedData.textures.emplace_back(name, uslot, 1, uslot);

	// Get texture format from source, if specified
	ResourceFormat::TextureFormat format = ResourceFormat::R8G8B8A8;
	const char* newLine = strchr(source, '\n');
	size_t lineLength = newLine - source;
	char* lineCopy = (char*)malloc(lineLength + 1);
	memset(lineCopy, '\0', lineLength + 1);
	strncpy_s(lineCopy, lineLength + 1, source, lineLength);

	if (strstr(lineCopy, "SAIL_NO_RESOURCE")) {
		// Skip renderable texture resource creation
		free(lineCopy);
		return;
	}

	if (strstr(lineCopy, "SAIL_RGBA16_FLOAT")) {
		format = ResourceFormat::R16G16B16A16_FLOAT;
	} else if (strstr(lineCopy, "SAIL_R8_UNORM")) {
		format = ResourceFormat::R8;
	} else if (strstr(lineCopy, "SAIL_")) {
		assert(false && "unknown format");
	}
	// TODO: support more formats
	free(lineCopy);

	std::string nameSuffix(" File: " + m_filename + " slot " + std::to_string(slot));
	m_parsedData.renderableTextures.emplace_back(ShaderResource(name, uslot, 1, uslot), format, nameSuffix);
}

std::string ShaderParser::nextTokenAsName(const char* source, UINT& outTokenSize, int* arrayElements) const {
	std::string name = nextToken(source);
	outTokenSize = (UINT)name.size() + 1U; /// +1 to account for the space before the name
	if (name[name.size() - 1] == ';') {
		name = name.substr(0, name.size() - 1); // Remove ending ';'
	}
	bool isArray = name[name.size() - 1] == ']';
	if (!arrayElements && isArray) {
		Logger::Error("Shader resource with name \"" + name + "\" is of unsupported type - array");
	}
	if (isArray) {
		// remove [asd] part from the name, iterativly to handle multidimensional arrays [asd][asd]
		size_t start = 0;
		while ((start = name.find("[")) != name.npos) {
			auto size = name.substr(start).find(']') + 1;
			if (size == 2) { // Sizeless / boundless array
				*arrayElements = -1;
				break;
			}
			// Add array size as long as the size is defined numerically and not with a macro
			if (isdigit(*name.substr(start+1).c_str()))
				*arrayElements *= std::stoi(name.substr(start+1, size-2));
			name.erase(start, size);
		}
	}
	return name;
}

std::string ShaderParser::nextTokenAsType(const char* source, UINT& outTokenSize) const {
	// Read until '>'
	outTokenSize = findToken(">", source) - source;
	std::string type;
	type.assign(source, std::find(source, source + outTokenSize, '\0'));
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
	if (typeName == "uint" ||
		typeName == "int"  ||
		typeName == "bool")								return 4;
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
	if (typeName == "PointLightInput")					return 4 * 8 * 128; // last 128 is NUM_POINT_LIGHTS
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
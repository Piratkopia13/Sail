#pragma once
#include <string>
#include "../material/Material.h"
#include "Sail/api/GraphicsAPI.h"
#include "Sail/api/shader/PipelineStateObject.h"

struct ComputeShaderSettings {
	float threadGroupXScale = 1.0f;
	float threadGroupYScale = 1.0f;
	float threadGroupZScale = 1.0f;
};
struct ShaderSettings {
	std::string filename;
	Material::Type materialType;
	PipelineStateObject::PSOSettings defaultPSOSettings;
	ComputeShaderSettings computeShaderSettings;
};

enum ShaderIdentifier {
	PBRMaterialShader,
	PhongMaterialShader,
	OutlineShader,
	CubemapShader,
	GenerateMipsComputeShader,
};
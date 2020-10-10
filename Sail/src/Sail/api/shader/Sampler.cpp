#include "pch.h"
#include "Sampler.h"

std::map<std::string, ShaderComponent::Sampler::ShaderInfo> ShaderComponent::Sampler::GetShaderSlotsMap() {
	return s_shaderSlots;
}

const std::map<std::string, ShaderComponent::Sampler::ShaderInfo> ShaderComponent::Sampler::s_shaderSlots = {
	// {Shader macro, {register slot, texture filter}}
	{"SAIL_SAMPLER_ANIS_WRAP", {0, Texture::ANISOTROPIC, Texture::WRAP}},
	{"SAIL_SAMPLER_LINEAR_CLAMP", {1, Texture::LINEAR, Texture::CLAMP}},
	{"SAIL_SAMPLER_POINT_CLAMP", {2, Texture::POINT, Texture::CLAMP}},
};

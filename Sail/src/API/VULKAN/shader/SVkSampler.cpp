#include "pch.h"
#include "SVkSampler.h"
#include "../SVkUtils.h"

namespace ShaderComponent {

	Sampler* Sampler::Create(Texture::ADDRESS_MODE addressMode, Texture::FILTER filter, BIND_SHADER bindShader, unsigned int slot) {
		return SAIL_NEW SVkSampler(addressMode, filter, bindShader, slot);
	}

	SVkSampler::~SVkSampler() {
		vkDestroySampler(m_context->getDevice(), m_textureSampler, nullptr);
	}

	void SVkSampler::bind() {
		//throw std::logic_error("The method or operation is not implemented.");
	}

	const VkSampler& SVkSampler::get() const {
		return m_textureSampler;
	}

	SVkSampler::SVkSampler(Texture::ADDRESS_MODE addressMode, Texture::FILTER filter, BIND_SHADER bindShader, unsigned int slot) {
		m_context = Application::getInstance()->getAPI<SVkAPI>();

		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = 16.0f;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f;

		VK_CHECK_RESULT(vkCreateSampler(m_context->getDevice(), &samplerInfo, nullptr, &m_textureSampler));
	}

}
#include "pch.h"
#include "SSAO.h"
#include "../Application.h"

SSAO::SSAO() {
	EventSystem::getInstance()->subscribeToEvent(Event::WINDOW_RESIZE, this);

	m_resScale = 1 / 2.f; // Half res

	auto* app = Application::getInstance();
	auto windowWidth = app->getWindow()->getWindowWidth();
	auto windowHeight = app->getWindow()->getWindowHeight();

	m_width = windowWidth * m_resScale;
	m_height = windowHeight * m_resScale;
	m_outputTexture = std::unique_ptr<RenderableTexture>(RenderableTexture::Create(m_width, m_height, "SSAO output ", ResourceFormat::R8));

	std::uniform_real_distribution<float> randomFloats(0.f, 1.f); // random floats between 0 - 1
	std::default_random_engine generator;
	for (unsigned int i = 0; i < 64; ++i) {
		glm::vec3 sample(
			randomFloats(generator) * 2.f - 1.f,
			randomFloats(generator) * 2.f - 1.f,
			randomFloats(generator)
		);
		sample = glm::normalize(sample);
		sample *= randomFloats(generator);
		float scale = (float)i / 64.f;
		scale = Utils::lerp(0.1f, 1.0f, scale * scale);
		sample *= scale;
		m_kernel.push_back(glm::vec4(sample, 0.0f));
	}

	for (unsigned int i = 0; i < 16; i++) {
		glm::vec4 noise(
			randomFloats(generator) * 2.f - 1.f,
			randomFloats(generator) * 2.f - 1.f,
			0.f,
			0.f);
		m_noise.push_back(noise);
	}
}

SSAO::~SSAO() {
	EventSystem::getInstance()->unsubscribeFromEvent(Event::WINDOW_RESIZE, this);
}

RenderableTexture* SSAO::getRenderTargetTexture() {
	return m_outputTexture.get();
}

unsigned int SSAO::getRenderTargetWidth() const {
	return m_width;
}

unsigned int SSAO::getRenderTargetHeight() const {
	return m_height;
}

const std::tuple<void*, unsigned int> SSAO::getKernel() const {
	return {(void*)m_kernel.data(), m_kernel.size() * sizeof(m_kernel[0])};
}

const std::tuple<void*, unsigned int> SSAO::getNoise() const {
	return { (void*)m_noise.data(), m_noise.size() * sizeof(m_noise[0]) };
}

bool SSAO::onEvent(Event& event) {
	auto resizeEvent = [&](WindowResizeEvent& event) {
		m_width = event.getWidth() * m_resScale;
		m_height = event.getHeight() * m_resScale;
		m_outputTexture->resize(m_width, m_height);

		return true;
	};
	EventHandler::HandleType<WindowResizeEvent>(event, resizeEvent);
	return true;
}

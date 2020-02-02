#pragma once

#include "Camera.h"
#include "Sail/events/Events.h"

class PerspectiveCamera : public Camera, public IEventListener {

public:
	PerspectiveCamera(float fov, float aspectRatio, float nearZ, float farZ) 
		: m_fov(glm::radians(fov))
		, m_aspectRatio(aspectRatio)
	{
		EventSystem::getInstance()->subscribeToEvent(Event::WINDOW_RESIZE, this);

		nearZDst = nearZ;
		farZDst = farZ;
		m_projectionMatrix = glm::perspectiveFovLH(m_fov, aspectRatio, 1.0f, nearZ, farZ);
	};
	~PerspectiveCamera() {
		EventSystem::getInstance()->unsubscribeFromEvent(Event::WINDOW_RESIZE, this);
	}

	void resize(int width, int height) {
		m_aspectRatio = static_cast<float>(width) / height;
		m_projectionMatrix = glm::perspectiveFovLH(m_fov, m_aspectRatio, 1.0f, nearZDst, farZDst);
	}

	float getFOV() const {
		return m_fov;
	}
	float getAspectRatio() const {
		return m_aspectRatio;
	}

	bool onEvent(Event& event) override {
		auto windowResize = [this](WindowResizeEvent& event) {
			resize(event.getWidth(), event.getHeight());
			return true;
		};
		EventHandler::HandleType<WindowResizeEvent>(event, windowResize);
		return true;
	}

private:
	virtual const glm::mat4& getProjectionMatrix() {
		return m_projectionMatrix;
	}

private:
	float m_fov;
	float m_aspectRatio;

	glm::mat4 m_projectionMatrix;

};

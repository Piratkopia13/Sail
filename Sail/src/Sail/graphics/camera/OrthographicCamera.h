#pragma once

#include "Camera.h"
#include "Sail/events/Events.h"

class OrthographicCamera: public Camera, public IEventListener {

public:
	OrthographicCamera(float width, float height, float nearZ, float farZ) {
		EventSystem::getInstance()->subscribeToEvent(Event::WINDOW_RESIZE, this);

		nearZDst = nearZ;
		farZDst = farZ;

		m_projectionMatrix = glm::orthoLH(-width / 2.0f, width / 2.0f, -height / 2.0f, height / 2.0f, nearZ, farZ);
	};
	~OrthographicCamera() {
		EventSystem::getInstance()->unsubscribeFromEvent(Event::WINDOW_RESIZE, this);
	}

	void resize(int width, int height) {
		m_projectionMatrix = glm::orthoLH(-width / 2.0f, width / 2.0f, -height / 2.0f, height / 2.0f, nearZDst, farZDst);
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

	glm::mat4 m_projectionMatrix;

};

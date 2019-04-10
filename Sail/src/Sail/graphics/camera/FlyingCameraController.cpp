#include "pch.h"
#include "FlyingCameraController.h"
#include "Sail/Application.h"

FlyingCameraController::FlyingCameraController(Camera* cam)
	: CameraController(cam)
{
	m_yaw = 90.f;
	m_pitch = 0.f;
	m_roll = 0.f;
}

void FlyingCameraController::setDirection(const glm::vec3 & dir) {
	m_pitch = glm::degrees(glm::asin(dir.y));
	m_yaw = glm::degrees(atan2(dir.x, -dir.z)) - 90.f;
	Logger::Log("Pitch: " + std::to_string(m_pitch));
	Logger::Log("Yaw: " + std::to_string(m_yaw));
}

void FlyingCameraController::lookAt(const glm::vec3& pos) {
	glm::vec3 dir = pos - getCameraPosition();
	dir = glm::normalize(dir);
	setDirection(dir);
}

void FlyingCameraController::update(float dt) {

	Application* app = Application::getInstance();

	//auto& kbState = app->getInput().getKeyboardState();
	//auto& gpState = app->getInput().getGamePadState(0);

	//float movementSpeed = dt * 5.f;
	//float lookSensitivityMouse = 0.1f;
	//float lookSensitivityController = 90.0f * 0.016f;

	//// Increase speed if shift or right trigger is pressed
	//if (kbState.LeftShift)
	//	movementSpeed *= 5.f;

	////
	//// Forwards / backwards motion
	////

	//// Gamepad
	//setCameraPosition(getCameraPosition() + getCameraDirection() * gpState.thumbSticks.leftY * movementSpeed);

	//// Keyboard
	//if (kbState.W)
	//	setCameraPosition(getCameraPosition() + getCameraDirection() * movementSpeed);
	//if (kbState.S)
	//	setCameraPosition(getCameraPosition() - getCameraDirection() * movementSpeed);

	////
	//// Side to side motion
	////

	//glm::vec3 right = getCameraDirection().Cross(glm::vec3::Up);
	//right.Normalize();
	//// Gamepad
	//setCameraPosition(getCameraPosition() - right * gpState.thumbSticks.leftX * movementSpeed);

	//// Keyboard
	//if (kbState.A)
	//	setCameraPosition(getCameraPosition() + right * movementSpeed);
	//if (kbState.D)
	//	setCameraPosition(getCameraPosition() - right * movementSpeed);

	////
	//// Up and down motion
	////

	//// Gamepad
	//setCameraPosition(getCameraPosition() + glm::vec3::Up * gpState.buttons.a * movementSpeed);
	//setCameraPosition(getCameraPosition() + glm::vec3::Down * gpState.buttons.x * movementSpeed);

	//// Keyboard
	//if (kbState.Space)
	//	setCameraPosition(getCameraPosition() + glm::vec3::Up * movementSpeed);
	//if (kbState.LeftControl)
	//	setCameraPosition(getCameraPosition() + glm::vec3::Down * movementSpeed);

	////
	//// Look around motion
	////

	//// Gamepad
	//m_pitch += gpState.thumbSticks.rightY * lookSensitivityController;
	//m_yaw -= gpState.thumbSticks.rightX * lookSensitivityController;

	//// Mouse input

	//// Toggle cursor capture on right click
	//if (app->getInput().wasJustPressed(Input::MouseButton::RIGHT)) {
	//	app->getInput().showCursor(app->getInput().isCursorHidden());
	//}

	//if (app->getInput().isCursorHidden()) {
	//	m_pitch -= (float)(app->getInput().getMouseDY()) * lookSensitivityMouse;
	//	m_yaw -= (float)(app->getInput().getMouseDX()) * lookSensitivityMouse;
	//}


	// Lock pitch to the range -89 - 89
	if (m_pitch >= 89)
		m_pitch = 89;
	else if (m_pitch <= -89)
		m_pitch = -89;

	// Lock yaw to the range 0 - 360
	if (m_yaw >= 360)
		m_yaw -= 360;
	else if (m_yaw <= 0)
		m_yaw += 360;

	glm::vec3 forwards(
		std::cos(glm::radians(m_pitch)) * std::cos(glm::radians(m_yaw)),
		std::sin(glm::radians(m_pitch)),
		std::cos(glm::radians(m_pitch)) * std::sin(glm::radians(m_yaw))
	);
	forwards = glm::normalize(forwards);

	setCameraDirection(forwards);

}

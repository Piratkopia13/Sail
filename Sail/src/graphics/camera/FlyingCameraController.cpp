#include "FlyingCameraController.h"
#include "../../api/Application.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

FlyingCameraController::FlyingCameraController(Camera* cam)
	: CameraController(cam)
{
	m_yaw = 90.f;
	m_pitch = 0.f;
	m_roll = 0.f;
}

void FlyingCameraController::setDirection(const DirectX::SimpleMath::Vector3 & dir) {
	m_pitch = XMConvertToDegrees(asin(dir.y));
	m_yaw = XMConvertToDegrees(atan2(dir.x, -dir.z)) - 90.f;
	Logger::Log("Pitch: " + std::to_string(m_pitch));
	Logger::Log("Yaw: " + std::to_string(m_yaw));
}

void FlyingCameraController::lookAt(const DirectX::SimpleMath::Vector3& pos) {
	Vector3 dir = pos - getCameraPosition();
	dir.Normalize();
	setDirection(dir);
}

void FlyingCameraController::update(float dt) {

	Application* app = Application::getInstance();

	auto& kbState = app->getInput().getKeyboardState();
	auto& gpState = app->getInput().getGamePadState(0);

	float movementSpeed = dt * 5.f;
	float lookSensitivityMouse = 0.1f;
	float lookSensitivityController = 90.0f * 0.016f;

	// Increase speed if shift or right trigger is pressed
	if (kbState.LeftShift)
		movementSpeed *= 5.f;

	//
	// Forwards / backwards motion
	//

	// Gamepad
	setCameraPosition(getCameraPosition() + getCameraDirection() * gpState.thumbSticks.leftY * movementSpeed);

	// Keyboard
	if (kbState.W)
		setCameraPosition(getCameraPosition() + getCameraDirection() * movementSpeed);
	if (kbState.S)
		setCameraPosition(getCameraPosition() - getCameraDirection() * movementSpeed);

	//
	// Side to side motion
	//

	Vector3 right = getCameraDirection().Cross(Vector3::Up);
	right.Normalize();
	// Gamepad
	setCameraPosition(getCameraPosition() - right * gpState.thumbSticks.leftX * movementSpeed);

	// Keyboard
	if (kbState.A)
		setCameraPosition(getCameraPosition() + right * movementSpeed);
	if (kbState.D)
		setCameraPosition(getCameraPosition() - right * movementSpeed);

	//
	// Up and down motion
	//

	// Gamepad
	setCameraPosition(getCameraPosition() + DirectX::SimpleMath::Vector3::Up * gpState.buttons.a * movementSpeed);
	setCameraPosition(getCameraPosition() + DirectX::SimpleMath::Vector3::Down * gpState.buttons.x * movementSpeed);

	// Keyboard
	if (kbState.Space)
		setCameraPosition(getCameraPosition() + DirectX::SimpleMath::Vector3::Up * movementSpeed);
	if (kbState.LeftControl)
		setCameraPosition(getCameraPosition() + DirectX::SimpleMath::Vector3::Down * movementSpeed);

	//
	// Look around motion
	//

	// Gamepad
	m_pitch += gpState.thumbSticks.rightY * lookSensitivityController;
	m_yaw -= gpState.thumbSticks.rightX * lookSensitivityController;

	// Mouse input

	// Toggle cursor capture on right click
	if (app->getInput().wasJustPressed(Input::MouseButton::RIGHT)) {
		app->getInput().showCursor(app->getInput().isCursorHidden());
	}

	if (app->getInput().isCursorHidden()) {
		m_pitch -= (float)(app->getInput().getMouseDY()) * lookSensitivityMouse;
		m_yaw -= (float)(app->getInput().getMouseDX()) * lookSensitivityMouse;
	}


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

	Vector3 forwards(
		std::cos(XMConvertToRadians(m_pitch)) * std::cos(XMConvertToRadians(m_yaw)),
		std::sin(XMConvertToRadians(m_pitch)),
		std::cos(XMConvertToRadians(m_pitch)) * std::sin(XMConvertToRadians(m_yaw))
	);
	forwards.Normalize();

	setCameraDirection(forwards);

}

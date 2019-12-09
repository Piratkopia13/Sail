#include "Sail/../../Sail/src/Network/NetworkModule.hpp"
#include "SplashScreenState.h"

#include "Sail.h"
#include "../libraries/imgui/imgui.h"

#include "Sail/entities/systems/render/BeginEndFrameSystem.h"
#include <string>
#include <Psapi.h>

#include "Sail/utils/GUISettings.h"

#include "Sail/graphics/shader/compute/AnimationUpdateComputeShader.h"
#include "Sail/graphics/shader/compute/ParticleComputeShader.h"

#define MULTI_THREADED_LOADING

SplashScreenState::SplashScreenState(StateStack& stack)
	: State(stack)
{
	m_input = Input::GetInstance();
	m_app = Application::getInstance();
	ResourceManager* rm = &m_app->getResourceManager();

	unsigned int byteSize = rm->getTextureByteSize();

	rm->loadTexture("splash_logo_smaller.tga");

	byteSize = rm->getTextureByteSize();

	rm->loadShaderSet<AnimationUpdateComputeShader>();
	rm->loadShaderSet<GBufferWireframe>();
	rm->setDefaultShader(&rm->getShaderSet<GBufferOutShader>());

#ifdef MULTI_THREADED_LOADING
	m_modelThread = m_app->pushJobToThreadPool([&](int id) {return loadModels(m_app); });
#else
	loadModels(m_app);
#endif

	//Sleep(5000000);
}
SplashScreenState::~SplashScreenState() {
	m_modelThread.get();
}

bool SplashScreenState::processInput(float dt) {
	return true;
}
bool SplashScreenState::update(float dt, float alpha) {
	return true;
}
bool SplashScreenState::render(float dt, float alpha) {
	return true;
}
bool SplashScreenState::renderImgui(float dt) {
	return true;
}
bool SplashScreenState::onEvent(const Event& event) {
	return true;
}

bool SplashScreenState::loadModels(Application* app) {

	ResourceManager* rm = &app->getResourceManager();

#ifdef MULTI_THREADED_LOADING
	std::future<bool> textureThread = m_app->pushJobToThreadPool([&](int id) {return loadTextures(m_app); });
#else
	loadTextures(m_app);
#endif

	//Sleep(1000000);		// Used for observing when the RAM spike happens

//#ifndef _DEBUG
	constexpr size_t UPPER_LIMIT_MB = 240;

	rm->loadModel("Doc.fbx");
	waitUntilMemoryIsBelow(MB_to_Byte(UPPER_LIMIT_MB));

	rm->loadModel("Torch.fbx");
	rm->loadModel("Tiles/RoomWall.fbx");
	rm->loadModel("Tiles/RoomDoor.fbx");
	rm->loadModel("Tiles/CorridorDoor.fbx");
	rm->loadModel("Tiles/CorridorWall.fbx");
	waitUntilMemoryIsBelow(MB_to_Byte(UPPER_LIMIT_MB));

	rm->loadModel("Tiles/RoomCeiling.fbx");
	rm->loadModel("Tiles/RoomFloor.fbx");
	rm->loadModel("Tiles/CorridorCorner.fbx");
	rm->loadModel("Tiles/RoomCorner.fbx");
	waitUntilMemoryIsBelow(MB_to_Byte(UPPER_LIMIT_MB));

	rm->loadModel("Clutter/Table.fbx");
	rm->loadModel("Clutter/Boxes.fbx");
	rm->loadModel("Clutter/MediumBox.fbx");
	rm->loadModel("Clutter/SquareBox.fbx");
	waitUntilMemoryIsBelow(MB_to_Byte(UPPER_LIMIT_MB));

	rm->loadModel("Clutter/Books1.fbx");
	rm->loadModel("Clutter/Screen.fbx");
	rm->loadModel("Clutter/Notepad.fbx");
	rm->loadModel("Clutter/Saftblandare.fbx");
	waitUntilMemoryIsBelow(MB_to_Byte(UPPER_LIMIT_MB));

	rm->loadModel("WaterPistol.fbx");
	rm->loadModel("boundingBox.fbx", &rm->getShaderSet<GBufferWireframe>());
	rm->loadModel("Clutter/Microscope.fbx");
	waitUntilMemoryIsBelow(MB_to_Byte(UPPER_LIMIT_MB));

	rm->loadModel("Clutter/CloningVats.fbx");
	waitUntilMemoryIsBelow(MB_to_Byte(UPPER_LIMIT_MB));

	rm->loadModel("Clutter/ControlStation.fbx");
	waitUntilMemoryIsBelow(MB_to_Byte(UPPER_LIMIT_MB));

	rm->loadModel("CleaningBot.fbx");

	rm->clearSceneData();


	//LEAVE THIS FOR A MULTITHREADED FUTURE
//#else
//
//	std::vector <std::future<bool>> modelThreads;
//	
//	modelThreads.push_back(m_app->pushJobToThreadPool([&](int id) {return rm->loadModel("Doc.fbx"); }));
//	modelThreads.push_back(m_app->pushJobToThreadPool([&](int id) {return rm->loadModel("candleExported.fbx"); }));
//	modelThreads.push_back(m_app->pushJobToThreadPool([&](int id) {return rm->loadModel("Tiles/tileFlat.fbx"); }));
//	modelThreads.push_back(m_app->pushJobToThreadPool([&](int id) {return rm->loadModel("Tiles/RoomWall.fbx"); }));
//	modelThreads.push_back(m_app->pushJobToThreadPool([&](int id) {return rm->loadModel("Tiles/tileDoor.fbx"); }));
//	modelThreads.push_back(m_app->pushJobToThreadPool([&](int id) {return rm->loadModel("Tiles/RoomDoor.fbx"); }));
//	modelThreads.push_back(m_app->pushJobToThreadPool([&](int id) {return rm->loadModel("Tiles/CorridorDoor.fbx"); }));
//	modelThreads.push_back(m_app->pushJobToThreadPool([&](int id) {return rm->loadModel("Tiles/CorridorWall.fbx"); }));
//	modelThreads.push_back(m_app->pushJobToThreadPool([&](int id) {return rm->loadModel("Tiles/RoomCeiling.fbx"); }));
//	modelThreads.push_back(m_app->pushJobToThreadPool([&](int id) {return rm->loadModel("Tiles/CorridorFloor.fbx"); }));
//	modelThreads.push_back(m_app->pushJobToThreadPool([&](int id) {return rm->loadModel("Tiles/RoomFloor.fbx"); }));
//	modelThreads.push_back(m_app->pushJobToThreadPool([&](int id) {return rm->loadModel("Tiles/CorridorCeiling.fbx"); }));
//	modelThreads.push_back(m_app->pushJobToThreadPool([&](int id) {return rm->loadModel("Tiles/CorridorCorner.fbx"); }));
//	modelThreads.push_back(m_app->pushJobToThreadPool([&](int id) {return rm->loadModel("Tiles/RoomCorner.fbx"); }));
//	modelThreads.push_back(m_app->pushJobToThreadPool([&](int id) {return rm->loadModel("Clutter/SmallObject.fbx"); }));
//	modelThreads.push_back(m_app->pushJobToThreadPool([&](int id) {return rm->loadModel("Clutter/MediumObject.fbx"); }));
//	modelThreads.push_back(m_app->pushJobToThreadPool([&](int id) {return rm->loadModel("Clutter/LargeObject.fbx"); }));
//
//
//
//	for (auto& modelThread : modelThreads) {
//		modelThread.get();
//	}
//#endif

#ifdef MULTI_THREADED_LOADING
	textureThread.get();
#endif

	SAIL_LOG("Waited for textures and models " + std::to_string(m_nrOfWaits) + " times");

	return true;
}
bool SplashScreenState::loadTextures(Application* app) {
	ResourceManager* rm = &app->getResourceManager();

	constexpr size_t UPPER_LIMIT_MB = 240;

	rm->loadTexture("pbr/DDS/Torch/Torch_MRAO.dds");
	rm->loadTexture("pbr/DDS/Torch/Torch_NM.dds");
	rm->loadTexture("pbr/DDS/Torch/Torch_Albedo.dds");

	waitUntilMemoryIsBelow(MB_to_Byte(UPPER_LIMIT_MB));

	rm->loadTexture("pbr/DDS/Tiles/RoomWallMRAO.dds");
	rm->loadTexture("pbr/DDS/Tiles/RoomWallNM.dds");
	rm->loadTexture("pbr/DDS/Tiles/RoomWallAlbedo.dds");

	waitUntilMemoryIsBelow(MB_to_Byte(UPPER_LIMIT_MB));

	rm->loadTexture("pbr/DDS/Tiles/RS_MRAo.dds");
	rm->loadTexture("pbr/DDS/Tiles/RS_NM.dds");
	rm->loadTexture("pbr/DDS/Tiles/RS_Albedo.dds");

	waitUntilMemoryIsBelow(MB_to_Byte(UPPER_LIMIT_MB));

	rm->loadTexture("pbr/DDS/Tiles/RD_MRAo.dds");
	rm->loadTexture("pbr/DDS/Tiles/RD_NM.dds");
	rm->loadTexture("pbr/DDS/Tiles/RD_Albedo.dds");

	waitUntilMemoryIsBelow(MB_to_Byte(UPPER_LIMIT_MB));

	rm->loadTexture("pbr/DDS/Tiles/CD_MRAo.dds");
	rm->loadTexture("pbr/DDS/Tiles/CD_NM.dds");
	rm->loadTexture("pbr/DDS/Tiles/CD_Albedo.dds");

	waitUntilMemoryIsBelow(MB_to_Byte(UPPER_LIMIT_MB));

	rm->loadTexture("pbr/DDS/Tiles/CW_MRAo.dds");
	rm->loadTexture("pbr/DDS/Tiles/CW_NM.dds");
	rm->loadTexture("pbr/DDS/Tiles/CW_Albedo.dds");

	waitUntilMemoryIsBelow(MB_to_Byte(UPPER_LIMIT_MB));

	rm->loadTexture("pbr/DDS/Tiles/F_MRAo.dds");
	rm->loadTexture("pbr/DDS/Tiles/F_NM.dds");
	rm->loadTexture("pbr/DDS/Tiles/F_Albedo.dds");

	waitUntilMemoryIsBelow(MB_to_Byte(UPPER_LIMIT_MB));

	rm->loadTexture("pbr/DDS/Tiles/CF_MRAo.dds");
	rm->loadTexture("pbr/DDS/Tiles/CF_NM.dds");
	rm->loadTexture("pbr/DDS/Tiles/CF_Albedo.dds");

	waitUntilMemoryIsBelow(MB_to_Byte(UPPER_LIMIT_MB));

	rm->loadTexture("pbr/DDS/Tiles/CC_MRAo.dds");
	rm->loadTexture("pbr/DDS/Tiles/CC_NM.dds");
	rm->loadTexture("pbr/DDS/Tiles/CC_Albedo.dds");

	waitUntilMemoryIsBelow(MB_to_Byte(UPPER_LIMIT_MB));

	rm->loadTexture("pbr/DDS/Tiles/RC_MRAo.dds");
	rm->loadTexture("pbr/DDS/Tiles/RC_NM.dds");
	rm->loadTexture("pbr/DDS/Tiles/RC_Albedo.dds");

	waitUntilMemoryIsBelow(MB_to_Byte(UPPER_LIMIT_MB));

	rm->loadTexture("pbr/DDS/Tiles/Corner_MRAo.dds");
	rm->loadTexture("pbr/DDS/Tiles/Corner_NM.dds");
	rm->loadTexture("pbr/DDS/Tiles/Corner_Albedo.dds");

	waitUntilMemoryIsBelow(MB_to_Byte(UPPER_LIMIT_MB));

	rm->loadTexture("pbr/DDS/Clutter/Saftblandare_MRAO.dds");
	rm->loadTexture("pbr/DDS/Clutter/Saftblandare_NM.dds");
	rm->loadTexture("pbr/DDS/Clutter/Saftblandare_Albedo.dds");

	waitUntilMemoryIsBelow(MB_to_Byte(UPPER_LIMIT_MB));

	rm->loadTexture("pbr/DDS/Clutter/Boxes_MRAO.dds");
	rm->loadTexture("pbr/DDS/Clutter/Boxes_NM.dds");
	rm->loadTexture("pbr/DDS/Clutter/Boxes_Albedo.dds");

	waitUntilMemoryIsBelow(MB_to_Byte(UPPER_LIMIT_MB));

	rm->loadTexture("pbr/DDS/Clutter/Table_MRAO.dds");
	rm->loadTexture("pbr/DDS/Clutter/Table_NM.dds");
	rm->loadTexture("pbr/DDS/Clutter/Table_Albedo.dds");

	waitUntilMemoryIsBelow(MB_to_Byte(UPPER_LIMIT_MB));

	rm->loadTexture("pbr/DDS/Clutter/Book_MRAO.dds");
	rm->loadTexture("pbr/DDS/Clutter/Book_NM.dds");
	rm->loadTexture("pbr/DDS/Clutter/Book1_Albedo.dds");
	rm->loadTexture("pbr/DDS/Clutter/Book2_Albedo.dds");

	waitUntilMemoryIsBelow(MB_to_Byte(UPPER_LIMIT_MB));

	rm->loadTexture("pbr/DDS/Clutter/SquareBox_MRAO.dds");
	rm->loadTexture("pbr/DDS/Clutter/SquareBox_NM.dds");
	rm->loadTexture("pbr/DDS/Clutter/SquareBox_Albedo.dds");

	waitUntilMemoryIsBelow(MB_to_Byte(UPPER_LIMIT_MB));

	rm->loadTexture("pbr/DDS/Clutter/MediumBox_MRAO.dds");
	rm->loadTexture("pbr/DDS/Clutter/MediumBox_NM.dds");
	rm->loadTexture("pbr/DDS/Clutter/MediumBox_Albedo.dds");

	waitUntilMemoryIsBelow(MB_to_Byte(UPPER_LIMIT_MB));

	rm->loadTexture("pbr/DDS/Clutter/Screen_Albedo.dds");
	rm->loadTexture("pbr/DDS/Clutter/Screen_MRAO.dds");
	rm->loadTexture("pbr/DDS/Clutter/Screen_NM.dds");

	waitUntilMemoryIsBelow(MB_to_Byte(UPPER_LIMIT_MB));

	rm->loadTexture("pbr/DDS/Clutter/Notepad_Albedo.dds");
	rm->loadTexture("pbr/DDS/Clutter/Notepad_MRAO.dds");
	rm->loadTexture("pbr/DDS/Clutter/Notepad_NM.dds");

	waitUntilMemoryIsBelow(MB_to_Byte(UPPER_LIMIT_MB));

	rm->loadTexture("pbr/DDS/Clutter/Microscope_Albedo.dds");
	rm->loadTexture("pbr/DDS/Clutter/Microscope_MRAO.dds");
	rm->loadTexture("pbr/DDS/Clutter/Microscope_NM.dds");

	waitUntilMemoryIsBelow(MB_to_Byte(UPPER_LIMIT_MB));

	rm->loadTexture("pbr/DDS/Clutter/CloningVats_Albedo.dds");
	rm->loadTexture("pbr/DDS/Clutter/CloningVats_MRAO.dds");
	rm->loadTexture("pbr/DDS/Clutter/CloningVats_NM.dds");

	waitUntilMemoryIsBelow(MB_to_Byte(UPPER_LIMIT_MB));

	rm->loadTexture("pbr/DDS/Clutter/ControlStation_Albedo.dds");
	rm->loadTexture("pbr/DDS/Clutter/ControlStation_MRAO.dds");
	rm->loadTexture("pbr/DDS/Clutter/ControlStation_NM.dds");

	waitUntilMemoryIsBelow(MB_to_Byte(UPPER_LIMIT_MB));

	rm->loadTexture("pbr/DDS/CleaningRobot/CleaningBot_Albedo.dds");
	rm->loadTexture("pbr/DDS/CleaningRobot/CleaningBot_NM.dds");
	rm->loadTexture("pbr/DDS/CleaningRobot/CleaningBot_MRAO.dds");

	waitUntilMemoryIsBelow(MB_to_Byte(UPPER_LIMIT_MB));

	rm->loadTexture("pbr/DDS/Doc/Doc_Albedo.dds");
	rm->loadTexture("pbr/DDS/Doc/Doc_MRAO.dds");
	rm->loadTexture("pbr/DDS/Doc/Doc_NM.dds");

	rm->loadTexture("pbr/DDS/WaterGun/Watergun_Albedo.dds");
	rm->loadTexture("pbr/DDS/WaterGun/Watergun_MRAO.dds");
	rm->loadTexture("pbr/DDS/WaterGun/Watergun_NM.dds");

	waitUntilMemoryIsBelow(MB_to_Byte(UPPER_LIMIT_MB));

	rm->loadTexture("particles/animFire.dds");
	rm->loadTexture("particles/animSmoke.dds");

	waitUntilMemoryIsBelow(MB_to_Byte(UPPER_LIMIT_MB));

	rm->loadTexture("Icons/TorchLeft.tga");
	rm->loadTexture("Icons/TorchThrow2.tga");
	rm->loadTexture("Icons/PlayersLeft.tga");
	rm->loadTexture("Icons/CantShootIcon1.tga");
	rm->loadTexture("Icons/CantShootIcon2.tga");

	// Load the missing texture texture
	rm->loadTexture("missing.tga");

	return true;
}

void SplashScreenState::waitUntilMemoryIsBelow(size_t size) {
	PROCESS_MEMORY_COUNTERS pmc;
	
	int waitCount = 0;

	while (true) {
		GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
		if (pmc.WorkingSetSize < size) {
			break;
		}
		else {
			{
				std::unique_lock<std::mutex> lock(m_waitMutex);
				m_nrOfWaits++;
			}

			// Prevents infinite waiting
			if (waitCount++ > 1'000'000) {
				break;
			}
			continue;
		}
	}
}

size_t SplashScreenState::MB_to_Byte(size_t sizeMB) {
	return sizeMB * 1024 * 1024;
}

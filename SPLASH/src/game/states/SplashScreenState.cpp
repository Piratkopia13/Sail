
#include "Sail/../../Sail/src/Network/NetworkModule.hpp"
#include "SplashScreenState.h"

#include "Sail.h"
#include "../libraries/imgui/imgui.h"

#include "Sail/entities/systems/render/BeginEndFrameSystem.h"
#include <string>



SplashScreenState::SplashScreenState(StateStack& stack)
	: State(stack)
{
	m_input = Input::GetInstance();
	m_app = Application::getInstance();


	m_modelThread = m_app->pushJobToThreadPool([&](int id) {return loadModels(m_app); });
}

SplashScreenState::~SplashScreenState() {


	m_modelThread.get();
}


bool SplashScreenState::processInput(float dt) {
	return false;
}

bool SplashScreenState::update(float dt, float alpha) {
	return false;
}

bool SplashScreenState::render(float dt, float alpha) {
	//m_app->getAPI()->clear({ 0.1f, 0.2f, 0.3f, 1.0f });
	//ECS::Instance()->getSystem<BeginEndFrameSystem>()->renderNothing();
	return false;
}




bool SplashScreenState::renderImgui(float dt) {

	return false;
}

bool SplashScreenState::onEvent(Event& event) {

	return false;
}


bool SplashScreenState::loadModels(Application* app) {
	ResourceManager* rm = &app->getResourceManager();
	rm->setDefaultShader(&app->getResourceManager().getShaderSet<GBufferOutShader>());
	std::future<bool> textureThread = m_app->pushJobToThreadPool([&](int id) {return loadTextures(m_app); });

//#ifndef _DEBUG
	rm->loadModel("Doc.fbx");
	rm->loadModel("candleExported.fbx");
	rm->loadModel("Tiles/tileFlat.fbx");
	rm->loadModel("Tiles/RoomWall.fbx"); 
	rm->loadModel("Tiles/tileDoor.fbx"); 
	rm->loadModel("Tiles/RoomDoor.fbx");
	rm->loadModel("Tiles/CorridorDoor.fbx");
	rm->loadModel("Tiles/CorridorWall.fbx"); 
	rm->loadModel("Tiles/RoomCeiling.fbx");
	rm->loadModel("Tiles/CorridorFloor.fbx");
	rm->loadModel("Tiles/RoomFloor.fbx");
	rm->loadModel("Tiles/CorridorCeiling.fbx");
	rm->loadModel("Tiles/CorridorCorner.fbx");
	rm->loadModel("Tiles/RoomCorner.fbx"); 
	rm->loadModel("Clutter/SmallObject.fbx");
	rm->loadModel("Clutter/MediumObject.fbx"); 
	rm->loadModel("Clutter/LargeObject.fbx");

	rm->loadModel("Clutter/Saftblandare.fbx");
	rm->loadModel("WaterPistol.fbx");
	rm->loadModel("boundingBox.fbx");
	rm->loadModel("cubeWidth1.fbx");

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


	textureThread.get();
	return true;
}

bool SplashScreenState::loadTextures(Application* app) {
	ResourceManager* rm = &app->getResourceManager();
	rm->loadTexture("pbr/Tiles/RoomWallMRAO.tga");
	rm->loadTexture("pbr/Tiles/RoomWallNM.tga");
	rm->loadTexture("pbr/Tiles/RoomWallAlbedo.tga");

	rm->loadTexture("pbr/Tiles/RD_MRAo.tga");
	rm->loadTexture("pbr/Tiles/RD_NM.tga");
	rm->loadTexture("pbr/Tiles/RD_Albedo.tga");

	rm->loadTexture("pbr/Tiles/CD_MRAo.tga");
	rm->loadTexture("pbr/Tiles/CD_NM.tga");
	rm->loadTexture("pbr/Tiles/CD_Albedo.tga");

	rm->loadTexture("pbr/Tiles/CW_MRAo.tga");
	rm->loadTexture("pbr/Tiles/CW_NM.tga");
	rm->loadTexture("pbr/Tiles/CW_Albedo.tga");

	rm->loadTexture("pbr/Tiles/F_MRAo.tga");
	rm->loadTexture("pbr/Tiles/F_NM.tga");
	rm->loadTexture("pbr/Tiles/F_Albedo.tga");

	rm->loadTexture("pbr/Tiles/CF_MRAo.tga");
	rm->loadTexture("pbr/Tiles/CF_NM.tga");
	rm->loadTexture("pbr/Tiles/CF_Albedo.tga");

	rm->loadTexture("pbr/Tiles/CC_MRAo.tga");
	rm->loadTexture("pbr/Tiles/CC_NM.tga");
	rm->loadTexture("pbr/Tiles/CC_Albedo.tga");

	rm->loadTexture("pbr/Tiles/RC_MRAo.tga");
	rm->loadTexture("pbr/Tiles/RC_NM.tga");
	rm->loadTexture("pbr/Tiles/RC_Albedo.tga");

	rm->loadTexture("pbr/Tiles/Corner_MRAo.tga");
	rm->loadTexture("pbr/Tiles/Corner_NM.tga");
	rm->loadTexture("pbr/Tiles/Corner_Albedo.tga");

	rm->loadTexture("pbr/metal/metalnessRoughnessAO.tga");
	rm->loadTexture("pbr/metal/normal.tga");
	rm->loadTexture("pbr/metal/albedo.tga");

	rm->loadTexture("pbr/Clutter/LO_MRAO.tga");
	rm->loadTexture("pbr/Clutter/LO_NM.tga");
	rm->loadTexture("pbr/Clutter/LO_Albedo.tga");

	rm->loadTexture("pbr/Clutter/MO_MRAO.tga");
	rm->loadTexture("pbr/Clutter/MO_NM.tga");
	rm->loadTexture("pbr/Clutter/MO_Albedo.tga");

	rm->loadTexture("pbr/Clutter/SO_MRAO.tga");
	rm->loadTexture("pbr/Clutter/SO_NM.tga");
	rm->loadTexture("pbr/Clutter/SO_Albedo.tga");

	rm->loadTexture("pbr/Clutter/Saftblandare_MRAO.tga");
	rm->loadTexture("pbr/Clutter/Saftblandare_NM.tga");
	rm->loadTexture("pbr/Clutter/Saftblandare_Albedo.tga");

	return false;
}
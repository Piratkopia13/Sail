#include "FBXParser.h"


FBXParser::FBXParser() {
	m_manager = FbxManager::Create();
	m_ios = FbxIOSettings::Create(m_manager, IOSROOT);
	m_manager->SetIOSettings(m_ios);
}

FBXParser::~FBXParser() {
	m_ios->Destroy();
	m_manager->Destroy();
}

FbxScene* FBXParser::parseFBX(const std::string& filename) {

	FbxImporter* importer = FbxImporter::Create(m_manager, "");

	if (!importer->Initialize(filename.c_str(), -1, m_manager->GetIOSettings())) 
		return nullptr;

	FbxScene* scene = FbxScene::Create(m_manager, "scene");
	importer->Import(scene);

	importer->Destroy();

	return scene;
}

FbxManager* FBXParser::getManager() {
	return m_manager;
}

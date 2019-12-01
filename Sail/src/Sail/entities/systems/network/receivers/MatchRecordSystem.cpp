#include "pch.h"
#include "MatchRecordSystem.h"
#include "Network/NWrapperSingleton.h"

MatchRecordSystem::MatchRecordSystem() {
	auto time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	auto time2 = std::ctime(&time);
}

MatchRecordSystem::~MatchRecordSystem() {
	recorded.close();
}

void MatchRecordSystem::initRecording() {
	if (status) {
		recorded.close();
		replay.close();
	}
	status = 1;
	recorded = std::ofstream("Nisse.txt"/*time2*/, std::ofstream::binary);

	const std::list<Player>& players = NWrapperSingleton::getInstance().getPlayers();
	unsigned len;
	unsigned np = (unsigned)players.size();
	recorded.write((char*)&np, sizeof(np));
	for (auto p : players) {
		len = (unsigned)p.name.length() + 1;
		recorded.write((char*)&p.id, 1);
		recorded.write((char*)&p.team, 1);
		recorded.write((char*)&len, sizeof(len));
		recorded.write(p.name.c_str(), len);
	}

	std::string settings = Application::getInstance()->getSettings().serialize(Application::getInstance()->getSettings().gameSettingsStatic, Application::getInstance()->getSettings().gameSettingsDynamic);
	len = (unsigned)settings.length();
	recorded.write((char*)&len, sizeof(len));
	recorded.write((char*)&settings[0], len);
}

void MatchRecordSystem::initReplay() {
	if (status) {
		recorded.close();
		replay.close();
	}
	status = 2;
	replay = std::ifstream("Nisse.txt"/*time2*/, std::ofstream::binary);

	unsigned np;
	unsigned strLen;
	std::string settings;

	NWrapperSingleton::getInstance().resetPlayerList();

	replay.read((char*)&np, sizeof(np));
	for (int i = 0; i < np; i++) {
		Player p;
		replay.read((char*)&p.id, 1);
		replay.read((char*)&p.team, 1);
		replay.read((char*)&strLen, sizeof(strLen));
		p.name.resize(strLen);
		replay.read(&p.name[0], strLen);

		NWrapperSingleton::getInstance().playerJoined(p);
		NWrapperSingleton::getInstance().getPlayer(p.id)->lastStateStatus.status = -1; //inform that this is not a real player
	}
	
	Player& myPlayer = NWrapperSingleton::getInstance().getMyPlayer();
	myPlayer.id = np;
	myPlayer.team = -1;
	NWrapperSingleton::getInstance().playerJoined(myPlayer);

	replay.read((char*)& strLen, sizeof(strLen));
	settings.resize(strLen);
	replay.read(&settings[0], strLen);

	Application::getInstance()->getSettings().deSerialize(settings, Application::getInstance()->getSettings().gameSettingsStatic, Application::getInstance()->getSettings().gameSettingsDynamic);
}

void MatchRecordSystem::recordPackage(const std::string& data) {
	unsigned np = 1;
	recorded.write((char*)& np, sizeof(np));

	for (size_t i = 0; i < np; i++) {
		unsigned pl = (unsigned)data.length();
		recorded.write((char*)& pl, sizeof(pl));
		recorded.write((char*)&data[0], pl);
	}
}

void MatchRecordSystem::replayPackages(std::queue<std::string>& data) {
	unsigned np;
	replay.read((char*)&np, sizeof(np));
	for (size_t i = 0; i < np; i++) {
		unsigned pl;
		std::string package;
		replay.read((char*)&pl, sizeof(pl));
		package.resize(pl);
		replay.read(&package[0], pl);
		data.push(package);
	}
}
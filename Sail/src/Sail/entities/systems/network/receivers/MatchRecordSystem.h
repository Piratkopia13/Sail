#pragma once
#include <fstream>
#include <chrono>
#include <ctime>    

class MatchRecordSystem {
public:
	MatchRecordSystem();
	~MatchRecordSystem();

	void initRecording();
	bool initReplay(std::string replayName);

	int status = 0;

	void recordPackages(std::queue<std::string> data);
	void replayPackages(std::queue<std::string>& data);

private:
	std::ofstream recorded;
	std::ifstream replay;
};
#pragma once
#include <fstream>
#include <chrono>
#include <ctime>    

class MatchRecordSystem {
public:
	MatchRecordSystem();
	~MatchRecordSystem();

	void initRecording();
	void initReplay();

	int status = 0;

	void recordPackage(const std::string& data);
	void replayPackages(std::queue<std::string>& data);

private:
	std::ofstream recorded;
	std::ifstream replay;
};
#include "pch.h"
#include "Profiler.h"
#include <Psapi.h>
#include "Sail/Application.h"

#include "Network/NWrapperSingleton.h"

Profiler::Profiler(bool showWindow) 
	: SailImGuiWindow(showWindow)
{
	init();
}

Profiler::Profiler() 
	: SailImGuiWindow(false)
{
	init();
}

Profiler::~Profiler() {
	delete[] m_virtRAMHistory;
	delete[] m_physRAMHistory;
	delete[] m_vramUsageHistory;
	delete[] m_cpuHistory;
	delete[] m_frameTimesHistory;
	delete[] m_fixedUpdateHistory;
	delete[] m_averageSentPacketSizeHistory;

	delete[] m_rmModelsSizeKBHistory;
	delete[] m_rmAnimationsSizeKBHistory;
	delete[] m_rmAudioSizeMBHistory;
	delete[] m_rmTexturesSizeMBHistory;
	delete[] m_rmGenericSizeBHistory;

#ifdef DEVELOPMENT
	delete[] m_ecsSizeKBHistory;
#endif
}

void Profiler::init() {
	SYSTEM_INFO sysInfo;
	FILETIME ftime, fsys, fuser;

	GetSystemInfo(&sysInfo);
	m_numProcessors = sysInfo.dwNumberOfProcessors;

	GetSystemTimeAsFileTime(&ftime);
	memcpy(&m_lastCPU, &ftime, sizeof(FILETIME));

	m_self = GetCurrentProcess();
	GetProcessTimes(m_self, &ftime, &ftime, &fsys, &fuser);
	memcpy(&m_lastSysCPU, &fsys, sizeof(FILETIME));
	memcpy(&m_lastUserCPU, &fuser, sizeof(FILETIME));

	// Allocating memory for profiler
	m_virtRAMHistory = SAIL_NEW float[100];
	m_physRAMHistory = SAIL_NEW float[100];
	m_vramUsageHistory = SAIL_NEW float[100];
	m_cpuHistory = SAIL_NEW float[100];
	m_frameTimesHistory = SAIL_NEW float[100];
	m_fixedUpdateHistory = SAIL_NEW float[100];
	m_averageSentPacketSizeHistory = SAIL_NEW float[100];

	m_rmModelsSizeKBHistory = SAIL_NEW float[100];
	m_rmAnimationsSizeKBHistory = SAIL_NEW float[100];
	m_rmAudioSizeMBHistory = SAIL_NEW float[100];
	m_rmTexturesSizeMBHistory = SAIL_NEW float[100];
	m_rmGenericSizeBHistory = SAIL_NEW float[100];

#ifdef DEVELOPMENT
	m_ecsSizeKBHistory = SAIL_NEW float[100];
#endif

	for (int i = 0; i < 100; i++) {
		m_virtRAMHistory[i] = 0.f;
		m_physRAMHistory[i] = 0.f;
		m_vramUsageHistory[i] = 0.f;
		m_cpuHistory[i] = 0.f;
		m_frameTimesHistory[i] = 0.f;
		m_fixedUpdateHistory[i] = 0.f;
		m_averageSentPacketSizeHistory[i] = 0.f;

		m_rmModelsSizeKBHistory[i] = 0.f;
		m_rmAnimationsSizeKBHistory[i] = 0.f;
		m_rmAudioSizeMBHistory[i] = 0.f;
		m_rmTexturesSizeMBHistory[i] = 0.f;
		m_rmGenericSizeBHistory[i] = 0.f;

#ifdef DEVELOPMENT
		m_ecsSizeKBHistory[i] = 0.f;
#endif
	}
}

unsigned int Profiler::workSetUsage() const{
	PROCESS_MEMORY_COUNTERS_EX pmc;
	GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
	SIZE_T physMemUsed = pmc.WorkingSetSize;
	
	return (int)(physMemUsed / (1024 * 1024));
}

unsigned int Profiler::virtMemUsage() const {
	PROCESS_MEMORY_COUNTERS_EX pmc;
	GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)& pmc, sizeof(pmc));
	SIZE_T virMemUsed = pmc.PrivateUsage;

	return (int)(virMemUsed / (1024 * 1024));
}

double Profiler::processUsage(){
	FILETIME ftime;
	FILETIME fsys;
	FILETIME fuser;
	ULARGE_INTEGER now;
	ULARGE_INTEGER sys;
	ULARGE_INTEGER user;
	double percent;

	GetSystemTimeAsFileTime(&ftime);
	memcpy(&now, &ftime, sizeof(FILETIME));

	GetProcessTimes(m_self, &ftime, &ftime, &fsys, &fuser);
	memcpy(&sys, &fsys, sizeof(FILETIME));
	memcpy(&user, &fuser, sizeof(FILETIME));
	percent = static_cast<double>((sys.QuadPart - m_lastSysCPU.QuadPart) + (user.QuadPart - m_lastUserCPU.QuadPart));
	percent /= (now.QuadPart - m_lastCPU.QuadPart);
	percent /= m_numProcessors;
	m_lastCPU = now;
	m_lastSysCPU = sys;
	m_lastUserCPU = user;

	if (percent > 0) {
		m_lastCPUpercent = percent;
	}

	return m_lastCPUpercent * 100;
}

unsigned int Profiler::vramUsage() const {
	return Application::getInstance()->getAPI()->getMemoryUsage();
}

unsigned int Profiler::vramBudget() const {
	return Application::getInstance()->getAPI()->getMemoryBudget();
}

void Profiler::renderWindow() {
	bool open = isWindowOpen();
	if (open) {
		if (ImGui::Begin("Profiler", &open)) {

			//Profiler window displaying the current usage
			showWindow(open);
			ImGui::BeginChild("Window", ImVec2(0, 0), false, 0);
			std::string header;

			header = "CPU (" + m_cpuCount + "%%)";
			ImGui::Text(header.c_str());

			header = "Frame time (" + m_ftCount + " s)";
			ImGui::Text(header.c_str());

			// "Potential" Hz in this case means what the update rate would be if the program consisted of
			// only the fixedUpdate() function running in a loop
			header = "FixedUpdate (" + m_fixedUpdateCount + " ms, " + m_potentialFixedUpdateRate + " \"potential\" Hz)";
			ImGui::Text(header.c_str());

			header = "Virtual RAM (" + m_virtCount + " MB)";
			ImGui::Text(header.c_str());

			header = "Physical RAM (" + m_physCount + " MB)";
			ImGui::Text(header.c_str());

			header = "VRAM (" + m_vramUCount + " MB)";
			ImGui::Text(header.c_str());

			ImGui::Separator();
			//Collapsing headers for graphs over time
			if (ImGui::CollapsingHeader("CPU Graph")) {
				header = "\n\n\n" + m_cpuCount + "(%)";
				ImGui::PlotLines(header.c_str(), m_cpuHistory, 100, 0, "", 0.f, 100.f, ImVec2(0, 100));
			}
			if (ImGui::CollapsingHeader("Frame Times Graph")) {
				header = "\n\n\n" + m_ftCount + "(s)";
				ImGui::PlotLines(header.c_str(), m_frameTimesHistory, 100, 0, "", 0.f, 0.015f, ImVec2(0, 100));
			}
			if (ImGui::CollapsingHeader("FixedUpdate Times Graph")) {
				header = "\n\n\n" + m_fixedUpdateCount + "(ms)";
				ImGui::PlotLines(header.c_str(), m_fixedUpdateHistory, 100, 0, "", 0.f, 0.015f, ImVec2(0, 100));
			}
			if (ImGui::CollapsingHeader("Virtual RAM Graph")) {
				header = "\n\n\n" + m_virtCount + "(MB)";
				ImGui::PlotLines(header.c_str(), m_virtRAMHistory, 100, 0, "", 0.f, 500.f, ImVec2(0, 100));
			}
			if (ImGui::CollapsingHeader("Physical RAM Graph")) {
				header = "\n\n\n" + m_physCount + "(MB)";
				ImGui::PlotLines(header.c_str(), m_physRAMHistory, 100, 0, "", 0.f, 500.f, ImVec2(0, 100));
			}
			if (ImGui::CollapsingHeader("VRAM Graph")) {
				header = "\n\n\n" + m_vramUCount + "(MB)";
				ImGui::PlotLines(header.c_str(), m_vramUsageHistory, 100, 0, "", 0.f, 500.f, ImVec2(0, 100));
			}
			if (ImGui::CollapsingHeader("Packet Size Graph")) {
				header = "\n\n\n" + m_averageSentPacketSize + "(B/s)";
				ImGui::PlotLines(header.c_str(), m_averageSentPacketSizeHistory, 100, 0, "", 0.f, 2000.f, ImVec2(0, 100));
			}
			if (ImGui::CollapsingHeader("Resource Manager")) {
				if (ImGui::CollapsingHeader("Models Graph")) {
					header = "\n\n\n" + m_rmModelsKB + "(KB)";
					ImGui::PlotLines(header.c_str(), m_rmModelsSizeKBHistory, 100, 0, "", 0.f, 2000.f, ImVec2(0, 100));
				}
				if (ImGui::CollapsingHeader("Animations Graph")) {
					header = "\n\n\n" + m_rmAnimationsKB + "(KB)";
					ImGui::PlotLines(header.c_str(), m_rmAnimationsSizeKBHistory, 100, 0, "", 0.f, 2000.f, ImVec2(0, 100));
				}
				if (ImGui::CollapsingHeader("Audio Graph")) {
					header = "\n\n\n" + m_rmAudioMB + "(MB)";
					ImGui::PlotLines(header.c_str(), m_rmAudioSizeMBHistory, 100, 0, "", 0.f, 200.f, ImVec2(0, 100));
				}
				if (ImGui::CollapsingHeader("Textures Graph")) {
					header = "\n\n\n" + m_rmTexturesMB + "(MB)";
					ImGui::PlotLines(header.c_str(), m_rmTexturesSizeMBHistory, 100, 0, "", 0.f, 500.f, ImVec2(0, 100));
				}
				if (ImGui::CollapsingHeader("Generic Graph")) {
					header = "\n\n\n" + m_rmGenericB + "(B)";
					ImGui::PlotLines(header.c_str(), m_rmGenericSizeBHistory, 100, 0, "", 0.f, 500.f, ImVec2(0, 100));
				}
			}
#ifdef DEVELOPMENT
			if (ImGui::CollapsingHeader("ECS Memory Graph")) {
				header = "\n\n\n" + m_ecsKB + "(kB)";
				ImGui::PlotLines(header.c_str(), m_ecsSizeKBHistory, 100, 0, "", 0.f, 500.f, ImVec2(0, 100));
			}
#endif

			ImGui::EndChild();

			float dt = Application::getInstance()->getDelta();
			float latestFixedUpdate = Application::getInstance()->getFixedUpdateDelta();


			m_profilerTimer += dt;
			constexpr float updateFrequency = 2.f;
			//Updating graphs and current usage
			if (m_profilerTimer > 1/updateFrequency) {
				m_profilerTimer = 0.f;

				size_t averagePacketSize = NWrapperSingleton::getInstance().averagePacketSizeSinceLastCheck();

				size_t rmModelsByteSize = Application::getInstance()->getResourceManager().getModelByteSize();
				size_t rmAnimationsByteSize = Application::getInstance()->getResourceManager().getAnimationsByteSize();
				size_t rmAudioByteSize = Application::getInstance()->getResourceManager().getAudioByteSize();
				size_t rmTexturesByteSize = Application::getInstance()->getResourceManager().getTextureByteSize();
				size_t rmGenericByteSize = Application::getInstance()->getResourceManager().getGenericByteSize();

#ifdef DEVELOPMENT
				const unsigned int ecsByteSize = ECS::Instance()->getByteSize();
#endif

				if (m_profilerCounter < 100) {

					m_virtRAMHistory[m_profilerCounter] = (float)virtMemUsage();
					m_physRAMHistory[m_profilerCounter] = (float)workSetUsage();
					m_vramUsageHistory[m_profilerCounter] = (float)vramUsage();
					m_frameTimesHistory[m_profilerCounter] = dt;
					m_fixedUpdateHistory[m_profilerCounter] = latestFixedUpdate;
					m_averageSentPacketSizeHistory[m_profilerCounter] = (float)averagePacketSize;

					m_rmModelsSizeKBHistory[m_profilerCounter] = (float)rmModelsByteSize / 1024.f;
					m_rmAnimationsSizeKBHistory[m_profilerCounter] = (float)rmAnimationsByteSize / 1024.f;
					m_rmAudioSizeMBHistory[m_profilerCounter] = (float)rmAudioByteSize / (1024.f * 1024.f);
					m_rmTexturesSizeMBHistory[m_profilerCounter] = (float)rmTexturesByteSize / (1024.f * 1024.f);
					m_rmGenericSizeBHistory[m_profilerCounter] = rmGenericByteSize;

#ifdef DEVELOPMENT
					m_ecsSizeKBHistory[m_profilerCounter] = (float)ecsByteSize / 1024.f;
#endif
					m_cpuHistory[m_profilerCounter++] = (float)processUsage();
					
					m_virtCount = std::to_string(virtMemUsage());
					m_physCount = std::to_string(workSetUsage());
					m_vramUCount = std::to_string(vramUsage());
					m_cpuCount = std::to_string(processUsage());
					m_ftCount = std::to_string(dt);
					m_fixedUpdateCount = std::to_string(latestFixedUpdate*1000.f);
					m_potentialFixedUpdateRate = std::to_string(static_cast<int>(1.0f / latestFixedUpdate));
					m_averageSentPacketSize = std::to_string(static_cast<size_t>(averagePacketSize * updateFrequency));

					m_rmModelsKB = std::to_string(static_cast<float>(rmModelsByteSize) / 1024.f);
					m_rmAnimationsKB = std::to_string(static_cast<float>(rmAnimationsByteSize) / 1024.f);
					m_rmAudioMB = std::to_string(static_cast<float>(rmAudioByteSize) / (1024.f * 1024.f));
					m_rmTexturesMB = std::to_string(static_cast<float>(rmTexturesByteSize) / (1024.f * 1024.f));
					m_rmGenericB = std::to_string(rmGenericByteSize);

#ifdef DEVELOPMENT
					m_ecsKB = std::to_string(static_cast<float>(ecsByteSize) / 1024.f);
#endif

				} else {
					// Copying all the history to a new array because ImGui is stupid

					auto copyHistory = [](float** arr, std::string& str, float value) {
						float* tempArr = SAIL_NEW float[100];
						std::copy(*arr + 1, *arr + 100, tempArr);
						tempArr[99] = value;
						delete[] *arr;
						*arr = tempArr;
						str = std::to_string(value);
					};

					copyHistory(&m_virtRAMHistory, m_virtCount, (float)virtMemUsage());
					
					copyHistory(&m_physRAMHistory, m_physCount, (float)workSetUsage());

					copyHistory(&m_vramUsageHistory, m_vramUCount, (float)vramUsage());

					copyHistory(&m_cpuHistory, m_cpuCount, (float)processUsage());

					copyHistory(&m_frameTimesHistory, m_ftCount, dt);

					float* tempFloatArr6 = SAIL_NEW float[100];
					std::copy(m_fixedUpdateHistory + 1, m_fixedUpdateHistory + 100, tempFloatArr6);
					tempFloatArr6[99] = latestFixedUpdate;
					delete m_fixedUpdateHistory;
					m_fixedUpdateHistory = tempFloatArr6;
					m_fixedUpdateCount = std::to_string(latestFixedUpdate*1000.f);
					m_potentialFixedUpdateRate = std::to_string(static_cast<int>(1.0f / latestFixedUpdate));

					float* tempFloatArr7 = SAIL_NEW float[100];
					std::copy(m_averageSentPacketSizeHistory + 1, m_averageSentPacketSizeHistory + 100, tempFloatArr7);
					tempFloatArr7[99] = (float)averagePacketSize;
					delete m_averageSentPacketSizeHistory;
					m_averageSentPacketSizeHistory = tempFloatArr7;
					m_averageSentPacketSize = std::to_string(static_cast<size_t>(averagePacketSize*updateFrequency));

					copyHistory(&m_rmModelsSizeKBHistory, m_rmModelsKB, (float)rmModelsByteSize / 1024.f);
					copyHistory(&m_rmAnimationsSizeKBHistory, m_rmAnimationsKB, (float)rmAnimationsByteSize / 1024.f);
					copyHistory(&m_rmAudioSizeMBHistory, m_rmAudioMB, (float)rmAudioByteSize / (1024.f * 1024.f));
					copyHistory(&m_rmTexturesSizeMBHistory, m_rmTexturesMB, (float)rmTexturesByteSize / (1024.f * 1024.f));
					copyHistory(&m_rmGenericSizeBHistory, m_rmGenericB, rmGenericByteSize);

#ifdef DEVELOPMENT
					copyHistory(&m_ecsSizeKBHistory, m_ecsKB, (float)ecsByteSize / 1024.f);
#endif
				}
			}
			ImGui::End();
		} else {
			ImGui::End();
		}
	}
}

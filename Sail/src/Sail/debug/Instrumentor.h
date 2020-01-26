//
// Basic instrumentation profiler by Cherno

// Usage: include this header file somewhere in your code (eg. precompiled header), and then use like:
//
// Instrumentor::Get().BeginSession("Session Name");        // Begin session 
// {
//     InstrumentationTimer timer("Profiled Scope Name");   // Place code like this in scopes you'd like to include in profiling
//     // Code
// }
// Instrumentor::Get().EndSession();                        // End Session
//
// You will probably want to macro-fy this, to switch on/off easily and use things like __FUNCSIG__ for the profile name.
//
#pragma once

#include <string>
#include <chrono>
#include <algorithm>
#include <fstream>
#include <thread>
#include <mutex>
#include <iostream>

//#define SAIL_PROFILING 1
//#define SAIL_PROFILING_APIo_SPECIFIC 1
#ifdef SAIL_PROFILING
	#define SAIL_PROFILE_BEGIN_SESSION(name, filepath) Instrumentor::Instance().beginSession(name, filepath)
	#define SAIL_PROFILE_END_SESSION() Instrumentor::Instance().endSession()
	#define SAIL_PROFILE_SCOPE(name) InstrumentationTimer timer##__LINE__(name)
	#define SAIL_PROFILE_FUNCTION() SAIL_PROFILE_SCOPE(__FUNCSIG__)
#ifdef SAIL_PROFILING_API_SPECIFIC
	#define SAIL_PROFILE_API_SPECIFIC_FUNCTION() SAIL_PROFILE_FUNCTION()
	#define SAIL_PROFILE_API_SPECIFIC_SCOPE(name) SAIL_PROFILE_SCOPE(name)
#else
	#define SAIL_PROFILE_API_SPECIFIC_FUNCTION()
#define SAIL_PROFILE_API_SPECIFIC_SCOPE(name)
#endif
#else
	#define SAIL_PROFILE_BEGIN_SESSION(name, filepath)
	#define SAIL_PROFILE_END_SESSION()
	#define SAIL_PROFILE_SCOPE(name)
	#define SAIL_PROFILE_FUNCTION()
	#define SAIL_PROFILE_API_SPECIFIC_FUNCTION()
	#define SAIL_PROFILE_API_SPECIFIC_SCOPE(name)
#endif

struct ProfileResult {
	const std::string name;
	long long start, end;
	uint32_t threadID;
};

class Instrumentor {
	std::string     m_sessionName = "None";
	std::ofstream   m_outputStream;
	int             m_profileCount = 0;
	std::mutex      m_lock;
	bool            m_activeSession = false;

	Instrumentor() { }

public:
	static Instrumentor& Instance() {
		static Instrumentor instance;
		return instance;
	}

	~Instrumentor() {
		endSession();
	}

	void beginSession(const std::string& name, const std::string& filepath = "results.json") {
		if (m_activeSession) { endSession(); }
		m_activeSession = true;
		m_outputStream.open("profiling/" + filepath);
		std::cout << "is open? " << filepath << m_outputStream.is_open() << std::endl;
		writeHeader();
		m_sessionName = name;
	}

	void endSession() {
		if (!m_activeSession) { return; }
		m_activeSession = false;
		writeFooter();
		m_outputStream.close();
		m_profileCount = 0;
	}

	bool replace(std::string& str, const std::string& from, const std::string& to) {
		size_t start_pos = str.find(from);
		if (start_pos == std::string::npos)
			return false;
		str.replace(start_pos, from.length(), to);
		return true;
	}

	void writeProfile(const ProfileResult& result) {
		std::lock_guard<std::mutex> lock(m_lock);

		if (m_profileCount++ > 0) { m_outputStream << ","; }

		std::string name = result.name;
		std::replace(name.begin(), name.end(), '"', '\'');
		
		// Clean up the name
		replace(name, "__cdecl ", "");
		replace(name, "(void)", "()");

		m_outputStream << "{";
		m_outputStream << "\"cat\":\"function\",";
		m_outputStream << "\"dur\":" << (result.end - result.start) << ',';
		m_outputStream << "\"name\":\"" << name << "\",";
		m_outputStream << "\"ph\":\"X\",";
		m_outputStream << "\"pid\":0,";
		m_outputStream << "\"tid\":" << result.threadID << ",";
		m_outputStream << "\"ts\":" << result.start;
		m_outputStream << "}";
	}

	void writeHeader() {
		m_outputStream << "{\"otherData\": {},\"traceEvents\":[";
	}

	void writeFooter() {
		m_outputStream << "]}";
	}
};

class InstrumentationTimer {
	ProfileResult m_result;

	std::chrono::time_point<std::chrono::high_resolution_clock> m_startTimepoint;
	bool m_stopped;

public:
	InstrumentationTimer(const std::string& name)
		: m_result({ name, 0, 0, 0 })
		, m_stopped(false) {
		m_startTimepoint = std::chrono::high_resolution_clock::now();
	}

	~InstrumentationTimer() {
		if (!m_stopped) { stop(); }
	}

	void stop() {
		auto endTimepoint = std::chrono::high_resolution_clock::now();

		m_result.start = std::chrono::time_point_cast<std::chrono::microseconds>(m_startTimepoint).time_since_epoch().count();
		m_result.end = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch().count();
		m_result.threadID = std::hash<std::thread::id>{}(std::this_thread::get_id());
		Instrumentor::Instance().writeProfile(m_result);

		m_stopped = true;
	}
};
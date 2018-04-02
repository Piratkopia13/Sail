#pragma once

#include <Windows.h>
#include <exception>
#include <iostream>
#include <string>
#include <sstream>
#include <random>
#include <d3d11.h>
#include <SimpleMath.h>

#define BIT(x) 1 << x

// Break on failed HRESULT return
#define ThrowIfFailed(result)	\
	if (FAILED(result))			\
		throw std::exception(); \

class Memory {

public:
	template <typename T>
	static void safeDelete(T& t) {
		if (t) {
			delete t;
			t = nullptr;
		}
	}

	template <typename T>
	static void safeDeleteArr(T& t) {
		if (t) {
			delete[] t;
			t = nullptr;
		}
	}

	template<typename T>
	static void safeRelease(T& t) {
		if (t) {
			t->Release();
			t = nullptr;
		}
	}

};

class Logger {

public:

	inline static void Log(const std::string& msg) {

		HANDLE hstdout = GetStdHandle(STD_OUTPUT_HANDLE);

		// Save currently set color
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		GetConsoleScreenBufferInfo(hstdout, &csbi);

		SetConsoleTextAttribute(hstdout, 0x0F);
		std::cout << "LOG: " << msg << std::endl;

		// Revert color
		SetConsoleTextAttribute(hstdout, csbi.wAttributes);


	}


	inline static void Warning(const std::string& msg) {
		
		HANDLE hstdout = GetStdHandle(STD_OUTPUT_HANDLE);

		// Save currently set color
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		GetConsoleScreenBufferInfo(hstdout, &csbi);

		SetConsoleTextAttribute(hstdout, 0xE0);
		std::cout << "WARNING: " << msg << std::endl;

		// Revert color
		SetConsoleTextAttribute(hstdout, csbi.wAttributes);

#ifdef _SAIL_BREAK_ON_WARNING
 		__debugbreak();
#endif
	}

	inline static void Error(const std::string& msg) {

		HANDLE hstdout = GetStdHandle(STD_OUTPUT_HANDLE);

		// Save currently set color
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		GetConsoleScreenBufferInfo(hstdout, &csbi);

		SetConsoleTextAttribute(hstdout, 0xC0);
		std::cout << "ERROR: " << msg << std::endl;

		// Revert color
		SetConsoleTextAttribute(hstdout, csbi.wAttributes);


#ifdef _SAIL_BREAK_ON_ERROR
		__debugbreak();
#endif
	}

};

namespace Utils {
	std::string readFile(const std::string& filepath);
	std::wstring vec3ToWStr(const DirectX::SimpleMath::Vector3& vec);
	std::string vec3ToStr(const DirectX::SimpleMath::Vector3& vec);
	float rnd();
	DirectX::SimpleMath::Vector4 getRandomColor();
	float clamp(float val, float min, float max);
	float smootherstep(float edge0, float edge1, float x);

	static std::random_device rd;
	static std::mt19937 gen(rd());
	static std::uniform_real_distribution<float> dis(0.f, 1.f);

	namespace String {
		std::string getBlockStartingFrom(const char* source);
		const char* findToken(const std::string& token, const char* source);
		int findNextIntOnLine(const char* source);
		const char* nextLine(const char* source);
		std::string nextToken(const char* source);
		const char* removeBeginningWhitespaces(const char* source);
		std::string removeComments(const std::string& source);
		bool startsWith(const char* source, const std::string& prefix);
	};

};
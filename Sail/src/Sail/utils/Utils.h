#pragma once

#include <exception>
#include <iostream>
#include <string>
#include <sstream>
#include <random>
#include <glm/glm.hpp>

#define BIT(x) 1 << x

// Break on failed HRESULT return
#define ThrowIfFailed(result)	\
	if (FAILED(result))			\
		throw std::exception();
// Show message box and break on failed blob creation
#define ThrowIfBlobError(hr, blob) { \
	if (FAILED(hr)) { \
		MessageBoxA(0, (char*)blob->GetBufferPointer(), "", 0); \
		OutputDebugStringA((char*)blob->GetBufferPointer()); \
		throw std::exception(); \
	} \
}

// Macro to easier track down memory leaks
#ifdef _DEBUG
#define SAIL_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#else
#define SAIL_NEW new
#endif

// Inherit this to make a class non-inheritable
template<typename T>
class MakeFinal {
private:
	~MakeFinal() { };
	friend T;
};

class Memory {

public:
	template <typename T>
	static void SafeDelete(T& t) {
		if (t) {
			delete t;
			t = nullptr;
		}
	}

	template <typename T>
	static void SafeDeleteArr(T& t) {
		if (t) {
			delete[] t;
			t = nullptr;
		}
	}

	template<typename T>
	static void SafeRelease(T& t) {
		if (t) {
			t->Release();
			t = nullptr;
		}
	}
	template<typename T>
	static void SafeRelease(T*& t) {
		if (t) {
			t->Release();
			t = nullptr;
		}
	}
	template<typename T>
	static void SafeRelease(T** t) {
		if (*t) {
			(*t)->Release();
			(*t) = nullptr;
		}
	}

};

class Logger {
public:
	static void Log(const std::string& msg);
	static void Warning(const std::string& msg);
	static void Error(const std::string& msg);
};

namespace Utils {
	std::string readFile(const std::string& filepath);
	std::vector<std::byte> readFileBinary(const std::string& filepath);
	std::wstring toWStr(const glm::vec3& vec);
	std::string toStr(const glm::vec4& vec);
	std::string toStr(const glm::vec3& vec);
	std::string toStr(const glm::vec2& vec);
	float rnd();
	glm::vec4 getRandomColor();
	float clamp(float val, float min, float max);
	float smootherstep(float edge0, float edge1, float x);
	float lerp(float a, float b, float f);

	static std::random_device rd;
	static std::mt19937 gen(rd());
	static std::uniform_real_distribution<float> dis(0.f, 1.f);

	namespace String {
		std::string getLineStartingFrom(const char* source);
		std::string getBlockStartingFrom(const char* source);
		const char* findToken(const std::string& token, const char* source, bool onFirstLine = false, bool ignoreBlocks = false); // ignoreBlocks: ignores characters enclosed in brackets "{..}"
		int findNextIntOnLine(const char* source);
		const char* nextLine(const char* source);
		std::string nextToken(const char* source);
		const char* removeBeginningWhitespaces(const char* source);
		std::string removeComments(const std::string& source);
		bool startsWith(const char* source, const std::string& prefix);
	};

	template < typename T>
	std::pair<bool, int> findInVector(const std::vector<T>& vecOfElements, const T& element) {
		std::pair<bool, int> result;

		// Find given element in vector
		auto it = std::find(vecOfElements.begin(), vecOfElements.end(), element);

		if (it != vecOfElements.end()) {
			result.second = distance(vecOfElements.begin(), it);
			result.first = true;
		} else {
			result.first = false;
			result.second = -1;
		}

		return result;
	}

};
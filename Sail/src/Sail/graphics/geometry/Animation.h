#pragma once
#include "Sail/utils/Utils.h"
#include <map>
#define SAIL_BONES_PER_VERTEX 4

class Animation {
public:
	class Frame {
	public:
		Frame();
		Frame(const unsigned int size);
		~Frame();
		void setTransform(const unsigned int index, const glm::mat4& transform);
		const unsigned int getTransformListSize();
		const glm::mat4* getTransformList();
	private:
		unsigned int m_transformSize;
		glm::mat4* m_limbTransform;
	};


	

	// TODO: rename behind variable to better describe it. (if frame behind or infront of time should be returned.)
	enum FindType {
		BEHIND,
		CLOSEST,
		INFRONT
	};

	Animation();
	~Animation();
	const float getMaxAnimationTime();
	const unsigned int getMaxAnimationFrame();
	const glm::mat4* getAnimationTransform(const float time, const FindType type = BEHIND);
	const glm::mat4* getAnimationTransform(const unsigned int frame);
	const unsigned int getAnimationTransformSize(const float time);
	const unsigned int getAnimationTransformSize(const unsigned int frame);
	const float getTimeAtFrame(const unsigned int frame);
	const unsigned int getFrameAtTime(const float time, const FindType type = BEHIND);
	void addFrame(const unsigned int frame, const float time, Animation::Frame* data);

	
private:
	
	float m_maxFrameTime;
	unsigned int m_maxFrame;

	inline const bool exists(const unsigned int frame);

	std::map<unsigned int, float> m_frameTimes;
	std::map<unsigned int, Animation::Frame*> m_frames;
};



class AnimationStack {
public:
	AnimationStack();
	AnimationStack(const unsigned int vertCount);
	~AnimationStack();
	
	//void setVertIndices(const int size, int* vertIndices);
	//void setLimbIndices
	void addAnimation(const std::string& animationName, Animation* animation);

	struct VertConnection {
		int count;
		int transform[SAIL_BONES_PER_VERTEX];
		float weight[SAIL_BONES_PER_VERTEX];

		VertConnection() {
			count = 0;
			for (int i = 0; i < SAIL_BONES_PER_VERTEX; i++) {
				transform[i] = -1;
				weight[i] = 0;
			}
		}
		void addConnection(const int _transform, const float _weight) {
#ifdef _DEBUG
			if (count >= SAIL_BONES_PER_VERTEX) {
				Logger::Error("AnimationStack:VertConnection: Too many existing connections(" + std::to_string(count) + ")");
			}
#endif
			if (count >= SAIL_BONES_PER_VERTEX) {
				return;
			}
			transform[count] = _transform;
			weight[count] = _weight;
			count++;
		}
		float checkWeights() {
			float sum = 0;
			for (int i = 0; i < count; i++) {
				sum += weight[i];
			}
			return sum;
		}
	};
	void setConnectionData(const unsigned int vertexIndex, const unsigned int boneIndex, float weight);


	void checkWeights() {
		for (unsigned int i = 0; i < m_connectionSize; i++) {
			float value = m_connections[i].checkWeights();
			if (value > 1.001 || value < 0.999) {
				Logger::Warning("Weights fucked: " + std::to_string(i)+ "(" + std::to_string(value)+")");
			}
		}
	}

private:

	unsigned int m_connectionSize;
	VertConnection* m_connections;
	std::vector<std::pair<std::string, Animation*>> m_stack;

};


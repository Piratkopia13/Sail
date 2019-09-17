#pragma once
#include "Sail/utils/Utils.h"
#include <map>
#define SAIL_BONES_PER_VERTEX 4

class Animation {
public:
	Animation();
	~Animation();


	
	class Frame {
	public:
		Frame();
		Frame(size_t size);
		~Frame();
		void setTransform(size_t index, const glm::mat4& transform) {
#ifdef _DEBUG
			if (index >= m_transformSize) {
				Logger::Error("Tried to add transform to index(" + std::to_string(index) + ") maxSize(" + std::to_string(m_transformSize));
				return;
			}
#endif
			m_limbTransform[index] = transform;
		};


	private:
		size_t m_transformSize;

		glm::mat4* m_limbTransform;
	};
	
	void pushBackFrame(float time, Animation::Frame& frame);
	void addFrame(Animation::Frame& frame);
	void addFrame(int* index, int* limb, float* limbWeight, glm::mat4* limbTransform, int indexSize, int limbSize, int transformSize);


	
private:


	std::vector<std::pair<float, Animation::Frame>> m_frames;

};



class AnimationStack {
public:
	AnimationStack();
	AnimationStack(const size_t vertCount);
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
			double sum = 0;
			for (int i = 0; i < count; i++) {
				sum += weight[i];
			}
			return sum;
		}
	};
	void setConnectionData(const int vertexIndex, const int boneIndex, float weight);


	void checkWeights() {
		for (int i = 0; i < m_connectionSize; i++) {
			float value = m_connections[i].checkWeights();
			if (value > 1.001 || value < 0.999) {
				Logger::Warning("Weights fucked: " + std::to_string(i)+ "(" + std::to_string(value)+")");
			}
		}
	}

private:

	size_t m_connectionSize;
	VertConnection* m_connections;
	std::vector<std::pair<std::string, Animation*>> m_stack;

};


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
		~Frame();
	
	private:
		int m_transformSize;

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
				return;
			}
#endif
			transform[count] = _transform;
			weight[count] = _weight;
			count++;
		}
		
	};
	void setConnectionData(const int vertexIndex, const int boneIndex, float weight);

	struct BoneInfo {
		size_t index;
		glm::mat4 offset;
	};
	std::map<std::string, AnimationStack::BoneInfo> m_boneMap;
private:

	size_t m_connectionSize;
	VertConnection* m_connections;
	std::vector<std::pair<std::string, Animation*>> m_stack;

};


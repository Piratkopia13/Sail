#pragma once



class Animation {
public:
	Animation() {};
	~Animation() {};


	
	class Frame {
	public:
		Frame() {};
		~Frame() {};
	
	private:
		int* index;
		int* limb;
		int* limbWeight;
		glm::mat3x3* limbTransform;
	};
	
	
private:
	std::vector<std::pair<float, Animation::Frame>> m_frames;

};



class AnimationStack {
public:
	AnimationStack() {};
	~AnimationStack() {};



private:
	std::vector<std::pair<std::string, Animation*>> m_stack;

};


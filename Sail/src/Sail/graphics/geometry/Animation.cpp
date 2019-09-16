#include "pch.h"
#include "Animation.h"

Animation::Animation() 
{

}

Animation::~Animation() {
}

void Animation::pushBackFrame(float time, Animation::Frame& frame) {
	m_frames.emplace_back(std::pair<float, Animation::Frame>(time, frame));
}

void Animation::addFrame(Animation::Frame& frame) {


}

void Animation::addFrame(int* index, int* limb, float* limbWeight, glm::mat4* limbTransform, int indexSize, int limbSize, int transformSize) {


}

AnimationStack::AnimationStack() {
}

AnimationStack::AnimationStack(const size_t vertCount) {
	m_connectionSize = vertCount;
	m_connections = new VertConnection[vertCount];
}

AnimationStack::~AnimationStack() {
	Memory::SafeDeleteArr(m_connections);
}

void AnimationStack::addAnimation(const std::string& animationName, Animation* animation) {

	m_stack.emplace_back(std::pair<std::string, Animation*>(animationName, animation));
}

void AnimationStack::setConnectionData(const int vertexIndex, const int boneIndex, float weight) {
#ifdef _DEBUG
	if (vertexIndex > m_connectionSize) {
		Logger::Error("AnimationStack::setBoneData: vertexIndex("+ std::to_string(vertexIndex) +") larger than array size("+std::to_string(m_connectionSize) +")." );
		return;
	}
#endif
	if (m_connections[vertexIndex].count >= 4) {
		Logger::Error("AnimationStack::SetConnectionData: vertexIndex: " + std::to_string(vertexIndex) + "");
	}
	m_connections[vertexIndex].addConnection(boneIndex, weight);
}

Animation::Frame::Frame() {
}

Animation::Frame::~Frame() {
}

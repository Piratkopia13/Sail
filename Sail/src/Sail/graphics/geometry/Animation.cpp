#include "pch.h"
#include "Animation.h"

#pragma region FRAME

Animation::Frame::Frame() :
	m_transformSize(0),
	m_limbTransform(nullptr) {
}
Animation::Frame::Frame(const unsigned int size) :
	m_transformSize(size) {
	m_limbTransform = SAIL_NEW glm::mat4[size];
	for (unsigned int i = 0; i < m_transformSize; i++) {
		m_limbTransform[i] = glm::identity<glm::mat4>();
	}
}
Animation::Frame::~Frame() {
	Memory::SafeDelete(m_limbTransform);
}
void Animation::Frame::setTransform(const unsigned int index, const glm::mat4& transform) {
#ifdef _DEBUG
	if (index >= m_transformSize || index < 0) {
		Logger::Error("Tried to add transform to index(" + std::to_string(index) + ") maxSize(" + std::to_string(m_transformSize));
		return;
	}
#endif
	m_limbTransform[index] = transform;

}
const unsigned int Animation::Frame::getTransformListSize() {
	return m_transformSize;
}
const glm::mat4* Animation::Frame::getTransformList() {
	return m_limbTransform;
}

#pragma endregion

#pragma region ANIMATION

Animation::Animation() :
	m_maxFrame(0) ,
	m_maxFrameTime(0)
{

}
Animation::~Animation() {
}
const float Animation::getMaxAnimationTime() {
	if(exists(m_maxFrame))
		return m_frameTimes[m_maxFrame];
	return 0.0f;
}
const unsigned int Animation::getMaxAnimationFrame() {
	return m_maxFrame;
}

const glm::mat4* Animation::getAnimationTransform(const float time, const FindType type) {
	const unsigned int frame = getFrameAtTime(time);
	return getAnimationTransform(frame);
}
const glm::mat4* Animation::getAnimationTransform(const unsigned int frame) {
	if (exists(frame))
		return m_frames[frame]->getTransformList();
	return nullptr;
}

const unsigned int Animation::getAnimationTransformSize(const float time) {
	const unsigned int frame = getFrameAtTime(time);
	return getAnimationTransformSize(frame);
}
const unsigned int Animation::getAnimationTransformSize(const unsigned int frame) {
	if (exists(frame))
		return m_frameTimes[frame];
	return 0;
}

const float Animation::getTimeAtFrame(const unsigned int frame) {
	if (exists(frame))
		return m_frameTimes[frame];
	return 0.0f;
}
const unsigned int Animation::getFrameAtTime(const float time, const FindType type) {
	// TODO: fmod
	if (time >= m_maxFrameTime)
		return 0;
	for (unsigned int frame = 0; frame < m_maxFrame; frame++) {
		float lastFrameTime = 0;
		if (exists(frame)) {
			if ((m_frameTimes[frame] == time))
				return frame;
			if (m_frameTimes[frame] > time) {

				if (type == BEHIND) {
					return frame - 1;
				}
				else if (type == INFRONT) {
					return frame;
				}
				else {
					float behind = time - m_frameTimes[frame - 1];
					float inFront = m_frameTimes[frame] - time;

					if (behind >= inFront)
						return frame - 1;
					else
						return frame;
				}
			}	
		}
	}
	return 0;
}

void Animation::addFrame(const unsigned int frame, const float time, Animation::Frame* data) {
	if (m_maxFrameTime < time)
		m_maxFrameTime = time;
	if (m_maxFrame < frame)
		m_maxFrame = frame;

	m_frames[frame] = data;
	m_frameTimes[frame] = time;
}

inline const bool Animation::exists(const unsigned int frame) {
	if (m_frames.find(frame) == m_frames.end()) {
		#ifdef _DEBUG
			Logger::Warning("Trying to access frame(" + std::to_string(frame) + ") which does not exist, maxFrame(" + std::to_string(m_maxFrame) + ").");
		#endif
		return false;
	}
	return true;
}

#pragma endregion

#pragma region VERTCONNECTION

AnimationStack::VertConnection::VertConnection() :
	count(0)
{
	for (unsigned int i = 0; i < SAIL_BONES_PER_VERTEX; i++) {
		transform[i] = 0;
		weight[i] = 0.0f;
	}
}

void AnimationStack::VertConnection::addConnection(const unsigned int _transform, const float _weight) {
	if (count >= SAIL_BONES_PER_VERTEX) {
		#ifdef _DEBUG
			Logger::Error("AnimationStack:VertConnection: Too many existing connections(" + std::to_string(count) + ")");
		#endif
		return;
	}
	transform[count] = _transform;
	weight[count] = _weight;
	count++;
}

const float AnimationStack::VertConnection::checkWeights() {
	float sum = 0;
	for (int i = 0; i < count; i++) {
		sum += weight[i];
	}
	return sum;
}



#pragma endregion

#pragma region ANIMATIONSTACK

AnimationStack::AnimationStack() {
}
AnimationStack::AnimationStack(const unsigned int vertCount) {
	m_connectionSize = vertCount;
	m_connections = new VertConnection[vertCount];
}
AnimationStack::~AnimationStack() {
	Memory::SafeDeleteArr(m_connections);
}
void AnimationStack::addAnimation(const std::string& animationName, Animation* animation) {
	if (m_stack.find(animationName) == m_stack.end()) {
		m_names[m_stack.size()] = animationName;
		m_stack[animationName] = animation;
	}
	else {
		Logger::Warning("Replacing animation " + animationName);
		m_stack[animationName] = animation;
	}
}
void AnimationStack::setConnectionData(const unsigned int vertexIndex, const unsigned int boneIndex, float weight) {
#ifdef _DEBUG
	if (vertexIndex > m_connectionSize) {
		Logger::Error("AnimationStack::setBoneData: vertexIndex("+ std::to_string(vertexIndex) +") larger than array size("+std::to_string(m_connectionSize) +")." );
		return;
	}
#endif
	if (m_connections[vertexIndex].count >= SAIL_BONES_PER_VERTEX) {
		Logger::Error("AnimationStack::SetConnectionData: vertexIndex: " + std::to_string(vertexIndex) + "");
	}
	m_connections[vertexIndex].addConnection(boneIndex, weight);
}

const Animation* AnimationStack::getAnimation(const std::string& name) {
	return nullptr;
}

void AnimationStack::checkWeights() {
	for (unsigned int i = 0; i < m_connectionSize; i++) {
		float value = m_connections[i].checkWeights();
		if (value > 1.001 || value < 0.999) {
			Logger::Warning("Weights fucked: " + std::to_string(i) + "(" + std::to_string(value) + ")");
		}
	}
}

#pragma endregion


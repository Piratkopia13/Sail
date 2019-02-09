#pragma once

#include <d3d11.h>
#include <SimpleMath.h>

class AABB {
public:
	AABB(const DirectX::SimpleMath::Vector3& minPos, const DirectX::SimpleMath::Vector3& maxPos);
	~AABB();

	void setMinPos(const DirectX::SimpleMath::Vector3& minPos);
	void setMaxPos(const DirectX::SimpleMath::Vector3& maxPos);
	const DirectX::SimpleMath::Vector3& getMinPos() const;
	const DirectX::SimpleMath::Vector3& getMaxPos() const;
	DirectX::SimpleMath::Vector3 getHalfSizes() const;
	DirectX::SimpleMath::Vector3 getCenterPos() const;
	void updateTransform(const DirectX::SimpleMath::Matrix& transform);
	void updateTranslation(const DirectX::SimpleMath::Vector3& translation);

	bool containsOrIntersects(const AABB& other);
	bool contains(const AABB& other);

private:
	bool lessThan(const DirectX::SimpleMath::Vector3& first, const DirectX::SimpleMath::Vector3& second);
	bool greaterThan(const DirectX::SimpleMath::Vector3& first, const DirectX::SimpleMath::Vector3& second);
	float& getElementByIndex(DirectX::SimpleMath::Vector3& vec, int index);

private:
	DirectX::SimpleMath::Vector3 m_minPos, m_originalMinPos;
	DirectX::SimpleMath::Vector3 m_maxPos, m_originalMaxPos;

};
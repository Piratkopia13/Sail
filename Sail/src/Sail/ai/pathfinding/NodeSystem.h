#pragma once

#include <map>
#include <list>

//Keep this define in case debugging is needed
//#define _DEBUG_NODESYSTEM

#ifdef _DEBUG_NODESYSTEM
#include "Sail/entities/Entity.h"
class Scene;
class Shader;
#endif

class Model;

/*
	Checks if container of type U<X> contains element of type X.

	Examples of usage:
	Can be used to see if std::list<int> contains a specific int.
	Can also be used to see if std::vector<char> contains a specific char.
	etc.
	etc.
*/
template<template<class T> class U, class X> bool contains(U<X>& container, X& element) {
	for (X& x : container) {
		if (x == element) {
			return true;
		}
	}
	return false;
};

class NodeSystem {
public:
	struct Node {
		glm::vec3 position;
		bool blocked;
		unsigned int index;
		Node(glm::vec3 pos, bool blocked, unsigned int i)
			: position(pos)
			, blocked(blocked)
			, index(i)
		{}
	};

	NodeSystem();
	~NodeSystem();

	void setNodes(const std::vector<Node>& nodes, const std::vector<std::vector<unsigned int>>& connections);

	std::vector<NodeSystem::Node> getPath(const NodeSystem::Node& from, const NodeSystem::Node& to);
	std::vector<NodeSystem::Node> getPath(const glm::vec3& from, const glm::vec3& to);

	const NodeSystem::Node& getNearestNode(const glm::vec3& position) const;
	unsigned int getDistance(unsigned int n1, unsigned int n2) const;

#ifdef _DEBUG_NODESYSTEM
	void setDebugModelAndScene(Shader* shader, Scene* scene);
	Model* m_nodeModel;
	Scene* m_scene;
	std::vector<Entity::SPtr> m_nodeEntities;
#endif

private:
		std::vector<unsigned int> BFS(const unsigned int from, const unsigned int to);
		std::vector<unsigned int> aStar(const unsigned int from, const unsigned int to);
		std::vector<std::vector<unsigned int>> m_connections;
		std::vector<NodeSystem::Node> m_nodes;
};
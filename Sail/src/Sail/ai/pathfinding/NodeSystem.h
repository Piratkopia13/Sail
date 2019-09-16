#pragma once

//#include "Sail.h"
#include <map>

class NodeSystem {
public:
	struct Node {
		glm::vec3 position;
	};

public:
	NodeSystem();
	~NodeSystem();

	void addNode(const Node& node, std::vector<unsigned int> connections);

	const Node& getNearestNode(const glm::vec3& position) const;
	const Node& getCurrentNode() const;
	const Node& getNextNode();

	void setCurrentNode(const Node& node);

private:
	unsigned int m_currentNode;

	std::vector<Node> m_nodes;
	std::map<unsigned int, std::vector<unsigned int>> m_connections;
};
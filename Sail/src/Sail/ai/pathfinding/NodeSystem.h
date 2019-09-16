#pragma once

//#include "Sail.h"
#include <map>
#include <list>

class NodeSystem {
public:
	struct Node {
		glm::vec3 position;
		unsigned int index;

		Node(glm::vec3 pos, unsigned int i)
			: position(pos)
			, index(i)
		{}
	};

private:
	class Graph {
	public:
		Graph(const unsigned int numNodes);
		~Graph();

		void addEdge(const unsigned int src, const unsigned int dest);
		const std::vector<unsigned int> BFS(const unsigned int from, const unsigned int to);

	private:
		std::vector<std::list<unsigned int>> m_connections;
	};

public:
	NodeSystem();
	~NodeSystem();

	void setNodes(const std::vector<glm::vec3>& nodes, const std::vector<std::vector<unsigned int>>& connections);

	const std::vector<NodeSystem::Node> getPath(const NodeSystem::Node& from, const NodeSystem::Node& to);
	const std::vector<NodeSystem::Node> getPath(const glm::vec3& from, const glm::vec3& to);

	const NodeSystem::Node& getNearestNode(const glm::vec3& position) const;

private:
	std::vector<NodeSystem::Node> m_nodes;

	NodeSystem::Graph* m_graph;
};
#include "pch.h"
#include "NodeSystem.h"
#include "Sail.h"

NodeSystem::NodeSystem() {
	m_graph = nullptr;
}

NodeSystem::~NodeSystem() {
	if ( m_graph ) {
		delete m_graph;
		m_graph = nullptr;
	}
}

void NodeSystem::setNodes(const std::vector<glm::vec3>& nodes, const std::vector<std::vector<unsigned int>>& connections) {
	if ( m_graph != nullptr ) {
		delete m_graph;
		m_graph = nullptr;
	}

	m_graph = new Graph(unsigned int(nodes.size()));

	for ( auto node : nodes ) {
		unsigned int index = unsigned int(m_nodes.size());
		m_nodes.emplace_back(node, index);
		for ( auto c : connections[index] ) {
			m_graph->addEdge(index, c);
			// m_graph.addEdge(c, index); <-- should probably be done
		}
	}
}

std::vector<NodeSystem::Node> NodeSystem::getPath(const NodeSystem::Node& from, const NodeSystem::Node& to) {
	auto path = m_graph->BFS(from.index, to.index);
	
	std::vector<NodeSystem::Node> nPath;
	for ( int i = path.size() - 1; i > -1; i-- ) {
		nPath.push_back(m_nodes[path[i]]);
	}

	return nPath;
}

std::vector<NodeSystem::Node> NodeSystem::getPath(const glm::vec3& from, const glm::vec3& to) {
	return getPath(getNearestNode(from), getNearestNode(to));
}

const NodeSystem::Node& NodeSystem::getNearestNode(const glm::vec3& position) const {
	float dist = FLT_MAX;
	unsigned int index = 0;

	for ( unsigned int i = 0; i < m_nodes.size(); i++ ) {
		float d = glm::distance(m_nodes[i].position, position);
		if ( d < dist ) {
			index = i;
			dist = d;
		}
	}

	return m_nodes[index];
}

/*
---------------
	 GRAPH
---------------
*/
NodeSystem::Graph::Graph(const unsigned int numNodes) {
	m_connections = std::vector<std::list<unsigned int>>(numNodes);
}

NodeSystem::Graph::~Graph() {}

void NodeSystem::Graph::addEdge(const unsigned int src, const unsigned int dest) {
	
	m_connections[src].push_back(dest);
}

std::vector<unsigned int> NodeSystem::Graph::BFS(const unsigned int from, const unsigned int to) {
	std::list<unsigned int> queue;
	std::vector<unsigned int> visited = std::vector<unsigned int>(m_connections.size());
	std::vector<unsigned int> path;
	std::vector<float> dist(m_connections.size(), FLT_MAX);
	std::vector<unsigned int> traceBack(m_connections.size(), INT_MAX);

	queue.emplace_back(from);
	visited[from] = true;
	dist[from] = 0;
	while ( !queue.empty() ) {
		unsigned int currNode = queue.front();
		queue.pop_front();

		for ( auto node : m_connections[currNode] ) {
			if ( !visited[node] ) {
				queue.push_back(node);
				visited[node] = true;
				traceBack[node] = currNode;
				dist[node] = dist[currNode] + 1;
			}
		}
	}

	unsigned int currNode = to;
	while ( currNode != from ) {
		if ( dist[currNode] != FLT_MAX ) {
			path.emplace_back(currNode);
			currNode = traceBack[currNode];
		}
		else {
			Logger::Warning("No path was found");
			return std::vector<unsigned int>(0);
		}
	}
	path.emplace_back(from);

	return path;
}

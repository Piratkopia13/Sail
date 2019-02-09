#pragma once

#include <vector>
#include <algorithm>
#include <functional>
#include "../../camera/Camera.h"
#include "../../shader/ShaderSet.h"
#include "AABB.h"
#include "../Model.h"
#include <memory>
#ifdef _DEBUG
	#include "../factory/CubeModel.h"
#endif

class Quadtree {
public:
	struct Element {
		Element(Model* model) : boundary(&model->getAABB()), model(model) {};
		Element(const Element& other)
			: boundary(other.boundary)
			, model(other.model)
		{}
		const AABB* boundary;
		Model* model;
	};

	class Node {
	public:
		Node(const AABB& boundary, unsigned int treeDepth = 0);
		~Node();
		bool insert(const Element& element);
		std::vector<Model*> query(const Frustum& frustum);

		void draw(Camera& cam, int& counter, std::function<void(ShaderSet*)> preDraw = std::function<void(ShaderSet*)>(), std::function<void(ShaderSet*)> postDraw = std::function<void(ShaderSet*)>());
		void drawSimple();

#ifdef _DEBUG
		std::vector<Model*> getNodesAsModels();
#endif

	private:
		void subdivide();

	private:
		const unsigned int MAX_ELEMENTS_IN_ONE_NODE = 4;

		AABB m_boundary;
		std::vector<std::unique_ptr<Quadtree::Element>> m_elements;
		Node* m_children[4];
		bool m_hasChildren;
		unsigned int m_treeDepth;
#ifdef _DEBUG
		std::unique_ptr<Model> m_model;
#endif
	};

public:
	Quadtree(const AABB& boundary);
	~Quadtree();

	Quadtree::Node& getRoot();

	static int numNodes;

private:
	Node m_root;
	static const unsigned int MAX_TREE_DEPTH;

};
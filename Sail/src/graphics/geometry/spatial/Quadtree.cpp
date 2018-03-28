#include "Quadtree.h"

using namespace DirectX;
using namespace SimpleMath;

int Quadtree::numNodes = 0;
const unsigned int Quadtree::MAX_TREE_DEPTH(5);

Quadtree::Quadtree(const AABB& boundary)
	: m_root(boundary)
{

}
Quadtree::~Quadtree() {

}


Quadtree::Node& Quadtree::getRoot() {
	return m_root;
}


Quadtree::Node::Node(const AABB& boundary, unsigned int treeDepth)
	: m_boundary(boundary)
	, m_treeDepth(treeDepth)
	, m_hasChildren(false)
{

	m_elements.reserve(MAX_ELEMENTS_IN_ONE_NODE + 1);

	for (unsigned int i = 0; i < 4; i++)
		m_children[i] = nullptr;

	numNodes++;

#ifdef _DEBUG
	/*m_model = ModelFactory::CubeModel::Create(m_boundary.getHalfSizes());
	m_model->getTransform().setTranslation(m_boundary.getCenterPos());*/
#endif
}

Quadtree::Node::~Node() {
	for (unsigned int i = 0; i < 4; i++)
		Memory::safeDelete(m_children[i]);
}

bool Quadtree::Node::insert(const Element& element) {

	// Return false if the element is not inside this node
	if (!m_boundary.contains(*element.boundary))
		return false;

	// Try to add the element to a child
	if (m_hasChildren) {
		for (unsigned int i = 0; i < 4; i++) {
			if (m_children[i]->insert(element))
				return true;
		}
		// Element did not fit into children, keep it here
		m_elements.push_back(std::make_unique<Element>(element));
	}

	// Insert the element into this node as long as the
	// max number of elements has not been reached
	if (m_treeDepth == MAX_TREE_DEPTH || m_elements.size() < MAX_ELEMENTS_IN_ONE_NODE) {
		m_elements.push_back(std::make_unique<Element>(element));
		return true;
	}

	// Subdivide into children
	if (!m_hasChildren && m_treeDepth < MAX_TREE_DEPTH) {
		// Add the element to this node
		// It will be moved to a child during subdivide if it fits
		m_elements.push_back(std::make_unique<Element>(element));
		subdivide();
		return true;
	}

	// Element did not fit anywhere in the tree
	return false;

}

void Quadtree::Node::draw(Camera& cam, int& counter, std::function<void(ShaderSet*)> preDraw, std::function<void(ShaderSet*)> postDraw) {

	// Dont do anything if this node is outside the frustum
	if (!cam.getFrustum().containsOrIntersects(m_boundary))
		return;

	for (auto& e : m_elements) {
		if (cam.getFrustum().containsOrIntersects(*e->boundary)) {
			e->model->getShader()->updateCamera(cam);

			if (preDraw) preDraw(e->model->getShader());
			//e->model->draw(); // TODO: fix
			if (postDraw) postDraw(e->model->getShader());

			counter++;
		}
	}

	if (m_hasChildren) {
		for (int i = 0; i < 4; i++) {
			m_children[i]->draw(cam, counter, preDraw, postDraw);
		}
	}

}

void Quadtree::Node::drawSimple() {

	for (auto& e : m_elements) {
		//e->model->draw(); // TODO: fix
	}

	if (m_hasChildren) {
		for (int i = 0; i < 4; i++) {
			m_children[i]->drawSimple();
		}
	}
}

void Quadtree::Node::subdivide() {

	// Create the children with boundaries

	const Vector3& minPos = m_boundary.getMinPos();
	const Vector3& maxPos = m_boundary.getMaxPos();
	Vector3 childSizes = m_boundary.getHalfSizes();
	// Dont change y height since this is a quadtree and not an octree
	childSizes.y *= 2.f;

	Vector3 childMinPos(minPos);
	Vector3 childMaxPos(childMinPos + childSizes);
	m_children[0] = new Quadtree::Node(AABB(childMinPos, childMaxPos), m_treeDepth + 1);

	childMinPos = Vector3(minPos.x + childSizes.x, minPos.y, minPos.z);
	childMaxPos = childMinPos + childSizes;
	m_children[1] = new Quadtree::Node(AABB(childMinPos, childMaxPos), m_treeDepth + 1);

	childMinPos = Vector3(minPos.x, minPos.y, minPos.z + childSizes.z);
	childMaxPos = childMinPos + childSizes;
	m_children[2] = new Quadtree::Node(AABB(childMinPos, childMaxPos), m_treeDepth + 1);

	childMinPos = Vector3(minPos.x + childSizes.x, minPos.y, minPos.z + childSizes.z);
	childMaxPos = childMinPos + childSizes;
	m_children[3] = new Quadtree::Node(AABB(childMinPos, childMaxPos), m_treeDepth + 1);

	// Remove elements from this vector if they can be inserted into a child
	for (unsigned int i = 0; i < 4; i++) {

		for (auto& it = m_elements.begin(); it != m_elements.end();){
			if (m_children[i]->insert(*it->get()))
				it = m_elements.erase(it);
			else
				++it;
		}

		/*m_elements.erase(std::remove_if(m_elements.begin(), m_elements.end(),
			[&](auto& e) { return m_children[i]->insert(*e); }), m_elements.end());*/
	}

	m_hasChildren = true;
}

std::vector<Model*> Quadtree::Node::query(const Frustum& frustum) {
	std::vector<Model*> output;
	output.reserve(MAX_ELEMENTS_IN_ONE_NODE);

	// Return empty list if node is not inside frustum
	if (!frustum.containsOrIntersects(m_boundary)) {
		return output;
	}

	// Append elements that are inside the frustum
	for (auto& e : m_elements) {
		if (frustum.containsOrIntersects(*e->boundary))
			output.push_back(e->model);
	}

	if (m_hasChildren) {
		// This node has children, append their elements
		for (int i = 0; i < 4; i++) {
			auto childElements = m_children[i]->query(frustum);
			output.insert(output.end(), childElements.begin(), childElements.end());
		}
	}
	
	return output;

}

#ifdef _DEBUG
std::vector<Model*> Quadtree::Node::getNodesAsModels() {
	std::vector<Model*> output;

	output.push_back(m_model.get());

	if (m_hasChildren) {
		// This node has children, append their elements
		for (unsigned int i = 0; i < 4; i++) {
			auto childModels = m_children[i]->getNodesAsModels();
			output.insert(output.end(), childModels.begin(), childModels.end());
		}
	}

	return output;
}
#endif
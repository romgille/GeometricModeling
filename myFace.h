#pragma once
#include <glm/glm.hpp>

class myVertex;
class myHalfedge;
class mySegment;

class myFace
{
public:
	myHalfedge *adjacent_halfedge;

	glm::vec3 normal;

	size_t index; //Extra variable. Use this variable as you wish!

	void computeNormal();

	size_t size() const;

	void copy(myFace *);
	void reset();

	glm::vec3 centroid() const;

	bool intersect(mySegment, float &, glm::mat4 model_matrix = glm::mat4(1.0f)) const;
	void closestVertexEdge(glm::vec3 p, myVertex * &, myHalfedge * &) const;

	myFace(myFace *);
	myFace();
	~myFace();
};
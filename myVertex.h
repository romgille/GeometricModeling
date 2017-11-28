#pragma once
#include <glm/glm.hpp>

class myHalfedge;

class myVertex
{
public:
	glm::vec3 point;
	myHalfedge *originof;

	glm::vec3 normal;
	size_t index;  //extra variable. Use as you wish!


	void computeNormal();

	void copy(myVertex *);
	void reset();

	myVertex(myVertex *);
	myVertex(glm::vec3 p = glm::vec3(0.0f, 0.0f, 0.0f)); 
	~myVertex();
};

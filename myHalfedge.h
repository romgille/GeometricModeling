#pragma once
#include <glm/glm.hpp>

class myVertex;
class myFace;

class myHalfedge
{
public:
	myVertex *source;
	myFace *adjacent_face;
	myHalfedge *next;
	myHalfedge *prev;
	myHalfedge *twin;

	size_t index; //Extra variable. Use as you wish!

	void copy(myHalfedge *);
	void reset();

	glm::vec3 edgeVertex();

	myHalfedge(myHalfedge *);
	myHalfedge();
	~myHalfedge();
};
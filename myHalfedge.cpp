#include "myHalfedge.h"
#include "myVertex.h"
#include "myFace.h"
#include "errors.h"
#include <glm/gtx/intersect.hpp>

myHalfedge::myHalfedge()
{
	reset();
}

void myHalfedge::copy(myHalfedge *ie)
{
	myassert(ie != nullptr);

	source = ie->source;
	adjacent_face = ie->adjacent_face;
	next = ie->next;
	prev = ie->prev;
	twin = ie->twin;
	index = ie->index;
}

void myHalfedge::reset()
{
	source = nullptr;
	adjacent_face = nullptr;
	next = nullptr;
	prev = nullptr;
	twin = nullptr;

	index = 0;
}

glm::vec3 myHalfedge::edgeVertex() {
	glm::vec3 centroid = this->adjacent_face->centroid();
	glm::vec3 centroid2 = this->twin->adjacent_face->centroid();
	glm::vec3 v0 = this->source->point;
	glm::vec3 v1 = this->twin->source->point;

	return (centroid + centroid2 + v0 + v1) / 4.0f;
}

myHalfedge::myHalfedge(myHalfedge *e)
{
	copy(e);
}

myHalfedge::~myHalfedge()
{

}
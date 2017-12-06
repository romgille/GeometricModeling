#include "myVertex.h"
#include "myHalfedge.h"
#include "myFace.h"
#include <glm/glm.hpp>
#include "errors.h"


myVertex::myVertex(myVertex *v)
{
	copy(v);
}

myVertex::myVertex(glm::vec3 p)
{
	reset();
	point = p;
}

myVertex::~myVertex()
{
}

void myVertex::reset()
{
	originof = nullptr;
	index = 0;
}

void myVertex::computeNormal()
{
	myassert(originof != nullptr);

	this->normal = glm::vec3(0.0f, 0.0f, 0.0f);
	myHalfedge *halfedge = this->originof;

	do
	{
		this->normal += halfedge->adjacent_face->normal;
		halfedge = halfedge->prev->twin;
	} while (halfedge != this->originof);

	this->normal = glm::normalize(this->normal);
}

glm::vec3 myVertex::vertexPoint() {
	glm::vec3 q = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 r = glm::vec3(0.0f, 0.0f, 0.0f);

	myHalfedge* tmp = this->originof;
	float i = 0;

	do {
		q += tmp->adjacent_face->centroid();
		r += (point + tmp->twin->source->point)/2.0f;
		tmp = tmp->twin->next;
		++i;
	} while (tmp != this->originof);
	q = q / i;
	r = r / i;

	return
		((1 / i) * q) + ((2 / i) * r) + (((i - 3) / i) * this->point);
}

void myVertex::copy(myVertex *v)
{
	myassert(v != nullptr);

	point = v->point;
	originof = v->originof;
	index = v->index;
}
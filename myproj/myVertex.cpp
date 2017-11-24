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
	myHalfedge *e = this->originof;
	
	do
	{
		this->normal = this->normal + e->adjacent_face->normal;
		e = e->prev->twin;
	} while (e != this->originof);
	
	this->normal = glm::normalize(this->normal);
}

void myVertex::copy(myVertex *v)
{
	myassert(v != nullptr);

	point = v->point;
	originof = v->originof;
	index = v->index;
}

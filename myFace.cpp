#include "myFace.h"
#include "myHalfedge.h"
#include "myVertex.h"
#include "mySegment.h"
#include <iostream>
#include <glm/gtx/intersect.hpp>
#include <algorithm>
#include "errors.h"

myFace::myFace(myFace *f)
{
	copy(f);
}

myFace::myFace()
{
	reset();
}

myFace::~myFace()
{
}

void myFace::reset()
{
	adjacent_halfedge = nullptr;
	index = 0;
}

void myFace::computeNormal()
{
	myassert(adjacent_halfedge != nullptr);

	glm::vec3 p1 = this->adjacent_halfedge->next->source->point;
	glm::vec3 p2 = this->adjacent_halfedge->source->point;
	glm::vec3 p3 = this->adjacent_halfedge->next->next->source->point;

	glm::vec3 vector1 = p3 - p1;
	glm::vec3 vector2 = p1 - p2;

	this->normal = glm::normalize(glm::cross(vector2, vector1));
}

size_t myFace::size() const
{
	myassert(adjacent_halfedge != nullptr);

	myHalfedge *e = adjacent_halfedge;
	size_t n = 0;
	do
	{
		n++;
		e = e->next;
	} while (e != adjacent_halfedge);
	return n;
}

void myFace::copy(myFace *f)
{
	myassert(f != nullptr);
	myassert(adjacent_halfedge != nullptr);

	adjacent_halfedge = f->adjacent_halfedge;
	index = f->index;
	normal = f->normal;
}

glm::vec3 myFace::centroid() const
{
	myassert(adjacent_halfedge != nullptr);

	myHalfedge* tmp = this->adjacent_halfedge;
	float nbVertex = 0;
	glm::vec3 centroid = glm::vec3(0.0f, 0.0f, 0.0f);

	do {
		centroid += tmp->source->point;
		tmp = tmp->next;
		++nbVertex;
	} while (tmp != this->adjacent_halfedge);

	return centroid / nbVertex;
}

bool myFace::intersect(mySegment ray, float & min_t, glm::mat4 model_matrix) const
{
	bool result = false;

	min_t = std::numeric_limits<float>::max();

	glm::vec4 tmp;
	glm::vec3 tmp_intersection_point;

	myHalfedge *e = adjacent_halfedge;
	glm::vec3 v1, v2, v3;

	tmp = model_matrix * glm::vec4(e->source->point, 1.0f);
	v1 = glm::vec3(tmp.x / tmp.w, tmp.y / tmp.w, tmp.z / tmp.w);

	tmp = model_matrix * glm::vec4(e->next->source->point, 1.0f);
	v2 = glm::vec3(tmp.x / tmp.w, tmp.y / tmp.w, tmp.z / tmp.w);

	e = e->next;
	do
	{
		tmp = model_matrix * glm::vec4(e->next->source->point, 1.0f);
		v3 = glm::vec3(tmp.x / tmp.w, tmp.y / tmp.w, tmp.z / tmp.w);

		bool intersect = glm::intersectRayTriangle(ray.p1, (ray.p2 - ray.p1), v1, v2, v3, tmp_intersection_point);
		if (intersect)
		{
			min_t = std::min(min_t, tmp_intersection_point.z);
			if (min_t >= 0) result = true;
		}

		v2 = v3;
		e = e->next;
	} while (e->next != adjacent_halfedge);

	return result;
}

void myFace::closestVertexEdge(glm::vec3 p, myVertex * & picked_vertex, myHalfedge * & picked_halfedge) const
{
	picked_vertex = nullptr;
	picked_halfedge = nullptr;

	float min_v = std::numeric_limits<float>::max();
	float min_e = std::numeric_limits<float>::max();

	myHalfedge *e = adjacent_halfedge;
	do
	{
		float t = glm::distance(p, e->source->point);
		if (t < min_v)
		{
			min_v = t;
			picked_vertex = e->source;
		}

		mySegment s(e->source->point, e->next->source->point);
		t = s.distance_squared(p);
		if (t < min_e)
		{
			min_e = t;
			picked_halfedge = e;
		}

		e = e->next;
	} while (e != adjacent_halfedge);
}

#include "myPlane3D.h"
#include "mySegment.h"


bool myPlane3D::intersect(mySegment &ray, float &t) const
{
	glm::vec3 v = ray.p2 - ray.p1;

	if (glm::dot(v, normal) == 0) 
		return false;

	t = - (glm::dot(ray.p1 - point, normal) / glm::dot(v, normal));
	
	return true;
}

myPlane3D::myPlane3D()
{
}


myPlane3D::~myPlane3D()
{
}

#pragma once
#include <glm/glm.hpp>

class mySegment;

class myPlane3D
{
public:
	glm::vec3 point;
	glm::vec3 normal;

	bool intersect(mySegment &, float &) const;

	myPlane3D();
	~myPlane3D();
};


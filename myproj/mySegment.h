#pragma once
#include <glm/glm.hpp>

class mySegment
{
public:
	mySegment();
	mySegment(glm::vec3, glm::vec3);
	~mySegment();

	glm::vec3 p1;
	glm::vec3 p2;

	//convention to represent lines, segments and rays is
	//segment: between p1 and p2
	//ray: starting at p1, in direction (p2 - p1)
	//line: passing through p1 and p2.

	float distance_squared(glm::vec3) const;
	float mySegment::length() const;
};


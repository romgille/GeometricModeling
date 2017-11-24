#include "mySegment.h"



mySegment::mySegment()
{
}

mySegment::mySegment(glm::vec3 _p1, glm::vec3 _p2)
	: p1(_p1), p2(_p2)
{
}


mySegment::~mySegment()
{
}

float mySegment::distance_squared(glm::vec3 q) const
{
	glm::vec3 direction = glm::normalize(p2 - p1);

	float p1_q_distance = glm::distance(p1, q);
	float p2_q_distance = glm::distance(p2, q);

	if (glm::dot(q - p1, direction) < 0)
		return p1_q_distance * p1_q_distance;

	else if (glm::dot(q - p2, -direction) < 0)
		return p2_q_distance * p2_q_distance;

	else
	{
		float dotp = glm::dot(q - p1, direction);
		return p1_q_distance * p1_q_distance - dotp * dotp;
	}
}

float mySegment::length() const
{
	return glm::distance(p1, p2);
}
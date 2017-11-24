#pragma once
#include <glm/glm.hpp>
#include <vector>

class mySegment;
class myFace;

class myAABB
{
public:
	enum cornerId { MIN, MAX };
	std::vector<glm::vec3> corner = { glm::vec3(), glm::vec3() };
	bool flag;

	void updateBox(myFace *);

	void reset();

	int largestDimension() const;

	bool intersect(mySegment ray, float &);

	float min_x() const;
	float min_y() const;
	float min_z() const;
	float max_x() const;
	float max_y() const;
	float max_z() const;
	
	myAABB();
	~myAABB();
};


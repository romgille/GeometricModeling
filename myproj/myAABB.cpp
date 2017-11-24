#include "myAABB.h"
#include "myFace.h"
#include "myHalfedge.h"
#include "myVertex.h"
#include <algorithm>   
#include "myPlane3D.h"
#include "mySegment.h"
#include <iostream>
#include "errors.h"

void myAABB::updateBox(myFace *f)
{
}

int myAABB::largestDimension() const
{
	return 0;
}

bool myAABB::intersect(mySegment ray, float &min_t)
{
	return false;
}

float myAABB::min_x() const
{
	return corner[MIN].x;
}

float myAABB::min_y() const
{
	return corner[MIN].y;
}

float myAABB::min_z() const
{
	return corner[MIN].z;
}

float myAABB::max_x() const
{
	return corner[MAX].x;
}

float myAABB::max_y() const
{
	return corner[MAX].y;
}

float myAABB::max_z() const
{
	return corner[MAX].z;
}

void myAABB::reset()
{
	corner[MIN] = glm::vec3(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
	corner[MAX] = glm::vec3(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());

	flag = false;
}

myAABB::myAABB()
{
	reset();
}


myAABB::~myAABB()
{
}

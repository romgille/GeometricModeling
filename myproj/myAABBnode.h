#pragma once
#include "myAABB.h"
#include <vector>

class myAABBnode
{
public:
	myAABB box;

	enum nodeID { LEFT, RIGHT };
	std::vector<myAABBnode *> child = { nullptr, nullptr };

	size_t face_index;
	size_t depth;

	bool isLeaf(void)
	{
		return (child[LEFT] == nullptr && child[RIGHT] == nullptr);
	}

	void reset();

	myAABBnode();
	~myAABBnode();
};


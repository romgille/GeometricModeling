#include "myBVH.h"
#include "myAABBnode.h"
#include <algorithm>
#include <stack>
#include <iostream>
#include "errors.h"

using namespace std;

void myBVH::buildBVH(myMesh *m, const splittingMethod s)
{
	reset();
}

void myBVH::_build(size_t left_index, size_t right_index, myAABBnode* root, size_t depth, const splittingMethod s)
{

}

bool myBVH::intersect(mySegment ray, myFace * & picked_face, float & min_t, glm::mat4 model_matrix)
{
	return false;
}

bool myBVH::intersect(mySegment ray, myFace * & picked_face, float & min_t, std::tuple<int, int, int, int> & stats, glm::mat4 model_matrix)
{
	return false;
}

size_t myBVH::splitWithPivot(const myAABB & box, const size_t left_index, const size_t right_index, const splittingMethod s)
{
	return 0;
}


void myBVH::reset()
{
	if (bvh_root != nullptr)
	{
		vector<myAABBnode *> all_nodes;
		stack<myAABBnode *> candidates;
		candidates.push(bvh_root);

		myAABBnode *current;
		while (!candidates.empty())
		{
			current = candidates.top(); candidates.pop();
			all_nodes.push_back(current);

			if (current->child[myAABBnode::LEFT] != nullptr) candidates.push(current->child[myAABBnode::LEFT]);
			if (current->child[myAABBnode::RIGHT] != nullptr) candidates.push(current->child[myAABBnode::RIGHT]);
		}

		for (size_t i = 0;i < all_nodes.size();i++)
			delete all_nodes[i];
	}

	for (size_t i = 0;i < all_bboxes.size();i++)
		all_bboxes[i].clear();
	all_bboxes.clear();
	sorted_allfaces.clear();

	bvh_root = nullptr;
}

myBVH::myBVH()
{
	bvh_root = nullptr;
}


myBVH::~myBVH()
{
	reset();
}

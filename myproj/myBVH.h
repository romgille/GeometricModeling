#pragma once
#include "myAABBnode.h"
#include "myMesh.h"

class myBVH
{
public:
	myAABBnode *bvh_root;
	std::vector<std::vector<myAABB>> all_bboxes;

	std::vector<myFace *> sorted_allfaces;

	enum splittingMethod { MIDPOINT, MEDIAN };

	void buildBVH(myMesh *, splittingMethod s = MIDPOINT);
	void _build(size_t left_index, size_t right_index, myAABBnode *root, size_t depth, splittingMethod);

	size_t splitWithPivot(const myAABB & box, const size_t left_index, const size_t right_index, const splittingMethod s);

	void reset();

	bool intersect(mySegment, myFace * &, float &, glm::mat4 modelview_matrix = glm::mat4(1.0f));
	bool intersect(mySegment, myFace * &, float &, std::tuple<int, int, int, int> &, glm::mat4 modelview_matrix = glm::mat4(1.0f));

	myBVH();
	~myBVH();
};


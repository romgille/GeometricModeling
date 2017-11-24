#include "myAABBnode.h"

 

myAABBnode::myAABBnode()
{
	reset();
}


myAABBnode::~myAABBnode()
{
}

void myAABBnode::reset()
{
	child[LEFT] = nullptr;
	child[RIGHT] = nullptr;
	face_index = 0;
	depth = 0;
}

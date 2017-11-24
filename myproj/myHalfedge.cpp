#include "myHalfedge.h"
#include "errors.h"

myHalfedge::myHalfedge()
{
	reset();
}

void myHalfedge::copy(myHalfedge *ie)
{
	myassert(ie != nullptr);

	source = ie->source;
	adjacent_face = ie->adjacent_face;
	next = ie->next;
	prev = ie->prev;
	twin = ie->twin;
	index = ie->index;
}

void myHalfedge::reset()
{
	source = nullptr;
	adjacent_face = nullptr;
	next = nullptr;
	prev = nullptr;
	twin = nullptr;

	index = 0;
}

myHalfedge::myHalfedge(myHalfedge *e)
{
	copy(e);
}

myHalfedge::~myHalfedge()
{

}

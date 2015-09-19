#include "GC.h"
#include "Cell.h"
#include "../env/Env.h"
namespace Opal { namespace GC {
;

static Object* _allObjs = nullptr;
static int _numObjs = 0;
static int _numLast = 0;

#define LOTS_OF_OBJECTS 64


Object::Object ()
	: gc_count(1), gc_marked(false)
{
	// collect first
	maybeCollect();

	// add to linked list
	gc_next = _allObjs;
	_allObjs = this;
	if (gc_next)
		gc_next->gc_prev = this;
	_numObjs++;
}
Object::~Object ()
{
	_numObjs--;

	// repair doubly linked list
	if (gc_next)
		gc_next->gc_prev = gc_prev;

	if (_allObjs == this)
		_allObjs = gc_next;
	else
		gc_prev->gc_next = gc_next;
}
void Object::mark ()
{
	// don't end up in a mark loop
	if (!gc_marked)
	{
		gc_marked = true;
		markChildren();
	}
}
void Object::markChildren () {}
void Object::retain () { gc_count++; }
bool Object::release () { return 0 == --gc_count; }



void collect ()
{
	for (auto& thread : Run::Thread::threads())
		for (size_t i = 0, len = thread->size(); i < len; i++)
			thread->get(i).mark();

	size_t ndeleted = 0;

	Object* obj, * next;
	for (obj = _allObjs; obj != nullptr; obj = next)
	{
		next = obj->gc_next;
		if (obj->gc_marked)
			obj->gc_marked = false;
		else
		{
			delete obj;
			ndeleted++;
		}
	}

//	if (ndeleted > 0)
//		std::cout << "[GC] collected " << ndeleted << " object(s)" << std::endl;
}
void maybeCollect ()
{
	if (_numObjs - _numLast > LOTS_OF_OBJECTS)
	{
		collect();
		_numLast = _numObjs;
	}
}
int numObjects () { return _numObjs; }


}}

#pragma once
#include "../opal.h"
namespace Opal {
namespace Env {
;
class Type;
class Function;
class Global;
}
namespace Run {
struct Cell;
}
namespace GC {
;

extern void collect ();
extern void maybeCollect ();
extern int numObjects ();


struct Object
{
	Object ();
	virtual ~Object ();
	void mark ();
	void retain ();
	bool release ();

	virtual void markChildren ();
private:
	friend void collect ();
	size_t gc_count;
	bool gc_marked;
	Object* gc_next;
	Object* gc_prev;
};



}}

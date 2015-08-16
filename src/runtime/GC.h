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

struct Object
{
	size_t gc_count;
	bool gc_marked;

	virtual ~Object ();
	void mark ();
	void retain ();
	void release ();

	virtual void markChildren ();
};



}}

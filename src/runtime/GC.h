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
	Object ();
	virtual ~Object ();
	void mark ();
	void retain ();
	bool release ();

	virtual void markChildren ();
private:
	size_t gc_count;
	bool gc_marked;
};



}}

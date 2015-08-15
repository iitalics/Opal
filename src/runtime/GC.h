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
	size_t count;
	virtual ~Object ();
	void mark ();
	void retain ();
	void release ();

	virtual void markChildren ();
};


}}

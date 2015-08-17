#include "GC.h"
#include "Cell.h"
#include "../env/Env.h"
namespace Opal { namespace GC {
;

// virtual
Object::~Object () {}
void Object::markChildren () {}

void Object::mark ()
{
	if (!gc_marked)
	{
		gc_marked = true;
		markChildren();
	}
}
void Object::retain () { gc_count++; }
void Object::release () { gc_count--; }


}}
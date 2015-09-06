#include "GC.h"
#include "Cell.h"
#include "../env/Env.h"
namespace Opal { namespace GC {
;




Object::Object ()
	: gc_count(1), gc_marked(false)
{}
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
bool Object::release () { return 0 == --gc_count; }


}}

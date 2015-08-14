#include "GC.h"
#include "Cell.h"
#include "../env/Env.h"
namespace Opal { namespace GC {
;

// virtual
Object::~Object () {}
void Object::markChildren () {}

// TODO: fill these in (garbage collector)
void Object::mark () {}
void Object::retain () {}
void Object::release () {}


}}

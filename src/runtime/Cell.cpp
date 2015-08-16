#include "Cell.h"
#include "GC.h"
#include "../env/Env.h"
namespace Opal { namespace Run {

static Env::Type* core_unit, *core_int, *core_bool, *core_real, *core_string;

void Cell::initTypes ()
{
	core_unit = Env::Type::core("unit");
	core_int = Env::Type::core("int");
	core_bool = Env::Type::core("bool");
	core_real = Env::Type::core("real");
	core_string = Env::Type::core("string");
}

// constructors
Cell Cell::Unit ()
{ return Cell { core_unit }; }
Cell Cell::Bool (bool b)
{ return Cell { core_bool, .dataBool = b }; }
Cell Cell::Int (Int_t n)
{ return Cell { core_int, .dataInt = n }; }
Cell Cell::Object (Env::Type* type, GC::Object* obj)
{ return Cell { type, .obj = obj }; }


void Cell::mark ()
{
	if (type->gc_collected && obj)
		obj->mark();
}
void Cell::retain ()
{
	if (type->gc_collected && obj)
		obj->retain();
}
void Cell::release ()
{
	if (type->gc_collected && obj)
	{
		obj->release();
		if (obj->gc_count == 0)
		{
			delete obj;
			obj = nullptr;
		}
	}
}


SimpleObject::SimpleObject (size_t nchilds)
{
	children.reserve(nchilds);
	for (size_t i = 0; i < nchilds; i++)
		children.push_back(Cell::Unit());
}
SimpleObject::~SimpleObject () { }

EnumObject::EnumObject (Env::Function* _ctor, size_t n)
	: SimpleObject(n), ctor(_ctor) {}
EnumObject::~EnumObject () {}


}}

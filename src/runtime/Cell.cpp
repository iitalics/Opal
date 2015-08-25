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
Cell Cell::Enum (Env::Type* type, size_t nfields, Env::Function* ctor)
{
	if (nfields > 0)
		return Cell::Object(type, new SimpleObject(nfields, ctor));
	else
		return Cell { type, .dataInt = 1 | Int_t(ctor) };
}


bool Cell::isEnum (Env::Function* ctor) const
{
	if (dataInt & 1)
		return (dataInt & ~1) == Int_t(ctor);
	else if (simple)
		return simple->ctor == ctor;
	else
		return false;
}
Cell& Cell::field (size_t i)
{
	return simple->children[i];
}

void Cell::mark ()
{
	if (type->gc_collected && obj)
		obj->mark();
}
Cell Cell::retain ()
{
	if (type->gc_collected && obj)
		obj->retain();
	return *this;
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


SimpleObject::SimpleObject (size_t nchilds, Env::Function* _ctor)
	: ctor(_ctor)
{
	children.reserve(nchilds);
	for (size_t i = 0; i < nchilds; i++)
		children.push_back(Cell::Unit());
}
SimpleObject::~SimpleObject () { }


}}

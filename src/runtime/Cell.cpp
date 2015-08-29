#include "Cell.h"
#include "GC.h"
#include "../env/Env.h"
namespace Opal { namespace Run {

static Env::Type* core_unit, *core_int, *core_bool, *core_char,
                 *core_real, *core_long, *core_string;

void Cell::initTypes ()
{
	core_unit = Env::Type::core("unit");
	core_int = Env::Type::core("int");
	core_real = Env::Type::core("real");
	core_long = Env::Type::core("long");
	core_bool = Env::Type::core("bool");
	core_char = Env::Type::core("char");
	core_string = Env::Type::core("string");
}

// constructors
Cell Cell::Unit ()
{ return Cell { core_unit, .obj = nullptr }; }
Cell Cell::Int (Int_t n)
{ return Cell { core_int, .dataInt = n }; }
Cell Cell::Real (Real_t n)
{ return Cell { core_real, .dataReal = n }; }
Cell Cell::Long (Long_t n)
{ return Cell { core_long, .dataLong = n }; }
Cell Cell::Bool (bool b)
{ return Cell { core_bool, .dataBool = b }; }
Cell Cell::Char (Char_t c)
{ return Cell { core_char, .dataChar = c }; }
Cell Cell::Object (Env::Type* type, GC::Object* obj)
{ return Cell { type, .obj = obj }; }
Cell Cell::String (const std::string& s)
{
	return Cell::Object(core_string,
		s.empty() ? nullptr : new StringObject(s));
}
Cell Cell::Enum (Env::Type* type, size_t nfields, Env::Function* ctor)
{
	return Cell::Object(type, new SimpleObject(nfields, ctor));
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

std::string Cell::str () const
{
	/*
		5
		5.0
		true/false
		"foo"
		<range_iter> [start = 0, end = 10]
	*/
	std::ostringstream ss;

	if (type == core_unit)
		return "()";
	else if (type == core_bool)
		return dataBool ? "true" : "false";
	else if (type == core_int)
		ss << dataInt;
	else if (type == core_long)
		ss << dataLong << "L";
	else if (type == core_char)
		ss << "\'" << dataChar << "\'";
	else if (type == core_real)
	{
		if (dataReal == Int_t(dataReal))
			ss << Int_t(dataReal) << ".0";
		else
			ss << dataReal;
	}
	else if (type == core_string)
	{
		if (obj == nullptr)
			ss << "\"\"";
		else
			ss << "\"" << ((StringObject*) obj)->string << "\"";
	}
	else if (type->isFunction())
	{
		ss << "<function> " << simple->ctor->fullname().str() << std::endl;
	}
	else
	{
		ss << "<" << type->name << ">";
		if (type->userCreate && type->data.nfields > 0)
		{
			ss << " [";
			for (size_t i = 0; i < type->data.nfields; i++)
			{
				if (i > 0)
					ss << ", ";
				ss << type->data.fields[i].name << " = "
				   << simple->get(i).str();
			}
			ss << "]";
		}
	}
	return ss.str();
}


SimpleObject::SimpleObject (size_t nchilds, Env::Function* _ctor)
	: ctor(_ctor)
{
	children.reserve(nchilds);
	for (size_t i = 0; i < nchilds; i++)
		children.push_back(Cell::Unit());
}
SimpleObject::~SimpleObject ()
{
	for (auto ch : children)
		ch.release();
}

Cell SimpleObject::get (size_t i)
{
	return children[i];
}
void SimpleObject::set (size_t i, Cell val)
{
	children[i].release();
	children[i] = val.retain();
}

StringObject::StringObject (const std::string& _string)
	: string(_string) {}
StringObject::~StringObject () {}

}}

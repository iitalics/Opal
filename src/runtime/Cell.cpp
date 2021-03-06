#include "Cell.h"
#include "GC.h"
#include "../env/Env.h"
namespace Opal { namespace Run {

static Env::Type* core_unit, *core_int, *core_bool, *core_char,
                 *core_real, *core_long, *core_box, *core_string;

void Cell::initTypes ()
{
	core_unit = Env::Type::core("unit");
	core_int = Env::Type::core("int");
	core_real = Env::Type::core("real");
	core_long = Env::Type::core("long");
	core_bool = Env::Type::core("bool");
	core_char = Env::Type::core("char");
	core_box = Env::Type::core(".box");
	core_string = Env::Type::core("string");
}

// constructors
Cell::Cell () : type(core_unit), obj(nullptr) {}

Cell Cell::Unit ()
{ return Cell(core_unit); }
Cell Cell::Int (Int_t n)
{
	auto res = Cell(core_int);
	res.dataInt = n;
	return res;
}
Cell Cell::Real (Real_t n)
{
	auto res = Cell(core_real);
	res.dataReal = n;
	return res;
}
Cell Cell::Long (Long_t n)
{
	auto res = Cell(core_long);
	res.dataLong = n;
	return res;
}
Cell Cell::Bool (bool b)
{
	auto res = Cell(core_bool);
	res.dataBool = b;
	return res;
}
Cell Cell::Char (Char_t c)
{
	auto res = Cell(core_char);
	res.dataChar = c;
	return res;
}
Cell Cell::Object (Env::Type* type, GC::Object* obj, Env::Function* ctor)
{
	auto res = Cell(type);
	res.obj = obj;
	res.ctor = ctor;
	return res;
}
Cell Cell::String (const std::string& s)
{
	return Cell::Object(core_string,
		s.empty() ? nullptr : new StringObject(s));
}
Cell Cell::Enum (Env::Type* type, size_t nfields, Env::Function* ctor)
{
	SimpleObject* obj;
	if (nfields > 0)
		obj = new SimpleObject(nfields);
	else
		obj = nullptr; // don't allocate if no fields
	return Cell::Object(type, obj, ctor);
}
Cell Cell::Box (const Cell& val)
{
	auto obj = new SimpleObject(1);
	obj->set(0, val);
	return Cell::Object(core_box, obj);
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
		if (obj->release())
		{
			delete obj;
			obj = nullptr;
		}
	}
}

std::string Cell::str (bool col) const
{
	/*
		5
		5.0
		true/false
		"foo"
		<range_iter> [start = 0, end = 10]
	*/
	std::ostringstream ss;

	bool uncolor = col;
	if (col)
	{
		if (type == core_int || type == core_long ||
				type == core_real || type == core_char ||
				type == core_bool)
			ss << "\x1b[33m";
		else if (type == core_string)
			ss << "\x1b[32m";
		else if (type->isFunction())
			ss << "\x1b[36m";
		else if (type->isTuple() || type == Env::Type::core("list"))
			uncolor = false;
		else
			ss << "\x1b[1m";
	}

	if (type == core_unit)
		ss << "()";
	else if (type == core_bool)
		ss << (dataBool ? "true" : "false");
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
			ss << "\"" << stringObj->string << "\"";
	}
	else if (type->isFunction())
	{
		ss << "<function> ";
		if (col)
			ss << "\x1b[0;1m";
		ss << ctor->fullname().str();
	}
	else if (type->isTuple())
	{
		ss << "(";
		for (size_t i = 0; i < type->nparams; i++)
		{
			if (i > 0)
				ss << ", ";
			ss << simple->get(i).str(col);;
		}
		ss << ")";
	}
	else if (type == Env::Type::core("list"))
	{
		ss << "[";
		auto obj = simple;
		while (obj)
		{
			auto hd = obj->get(0);
			auto tl = obj->get(1);

			ss << hd.str(col);
			if (tl.obj)
				ss << ", ";

			obj = tl.simple;
		}
		ss << "]";
	}
	else if (ctor != nullptr)
	{
		ss << ctor->fullname().str();
		if (col)
		{
			ss << "\x1b[0m";
			uncolor = false;
		}
		ss << "(";
		for (size_t i = 0; i < ctor->args.size(); i++)
		{
			if (i > 0)
				ss << ", ";
			ss << simple->get(i).str(col);
		}
		ss << ")";
	}
	else
	{
		ss << "<" << type->name << ">";
		if (col)
		{
			ss << "\x1b[0m";
			uncolor = false;
		}
		if (type->userCreate && type->data.nfields > 0)
		{
			ss << " [";
			for (size_t i = 0; i < type->data.nfields; i++)
			{
				if (i > 0)
					ss << ", ";
				ss << type->data.fields[i].name << " = "
				   << simple->get(i).str(col);
			}
			ss << "]";
		}
	}

	if (uncolor)
		ss << "\x1b[0m";
	return ss.str();
}


SimpleObject::SimpleObject (size_t nchilds)
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
void SimpleObject::markChildren ()
{
	for (auto cell : children) cell.mark();
}

StringObject::StringObject (const std::string& _string)
	: string(_string) {}
StringObject::~StringObject () {}

}}

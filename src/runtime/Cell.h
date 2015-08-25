#pragma once
#include "../opal.h"
#include "GC.h"
namespace Opal {
namespace Env {
class Type;
class Function;
class Global;
}
namespace Run {
;

struct SimpleObject;

struct Cell
{
	Env::Type* type;
	union
	{
		Int_t dataInt;
		Long_t dataLong;
		Real_t dataReal;
		Char_t dataChar;
		bool dataBool;
		GC::Object* obj;
		SimpleObject* simple;
		void* data;
	};

	void mark ();
	Cell retain ();
	void release ();

	static void initTypes ();
	static Cell Unit ();
	static Cell Bool (bool b);
	static Cell Int (Int_t n);
	static Cell Real (Real_t n);
	static Cell Long (Long_t n);
	static Cell Object (Env::Type* type, GC::Object* obj);
	static Cell Enum (Env::Type* type, size_t nfields,
		Env::Function* ctor = nullptr);

	bool isEnum (Env::Function* ctor) const;

	std::string str () const;
};

struct SimpleObject : public GC::Object
{
	SimpleObject (size_t nchilds, Env::Function* ctor = nullptr);
	virtual ~SimpleObject ();

	Env::Function* ctor;
	std::vector<Cell> children;

	Cell get (size_t i);
	void set (size_t i, Cell c);
};

}}

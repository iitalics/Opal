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
struct StringObject;

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
		struct
		{
			// union-ception
			union
			{
				void* data;
				GC::Object* obj;
				SimpleObject* simple;
				StringObject* stringObj;
			};

			Env::Function* ctor;
		};
	};

	Cell ();

	void mark ();
	Cell retain ();
	void release ();

	static void initTypes ();
	static Cell Unit ();
	static Cell Int (Int_t n);
	static Cell Real (Real_t n);
	static Cell Long (Long_t n);
	static Cell Bool (bool b);
	static Cell Char (Char_t c);
	static Cell String (const std::string& s);
	static Cell Object (Env::Type* type, GC::Object* obj,
		Env::Function* ctor = nullptr);
	static Cell Enum (Env::Type* type, size_t nfields,
		Env::Function* ctor = nullptr);
	static Cell Box (const Cell& val);

	std::string str () const;

private:
	inline Cell (Env::Type* _type)
		: type(_type), obj(nullptr) {}
};

struct SimpleObject : public GC::Object
{
	explicit SimpleObject (size_t nchilds);
	virtual ~SimpleObject ();
	virtual void markChildren ();

	std::vector<Cell> children;

	Cell get (size_t i);
	void set (size_t i, Cell c);
};

struct StringObject : public GC::Object
{
	StringObject (const std::string& str);
	virtual ~StringObject ();

	std::string string;
};

}}

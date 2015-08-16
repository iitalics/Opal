#pragma once
#include "../opal.h"
#include "GC.h"
namespace Opal {
namespace Env {
;
class Type;
class Function;
class Global;
}
namespace Run {

struct SimpleObject;
struct EnumObject;

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
		void* data;
	};

	void mark ();
	void retain ();
	void release ();

	static void initTypes ();
	static Cell Unit ();
	static Cell Bool (bool b);
	static Cell Int (Int_t n);
	static Cell Long (Long_t n);
	static Cell Object (Env::Type* type, GC::Object* obj);
};

struct SimpleObject : public GC::Object
{
	SimpleObject (size_t nchilds);
	virtual ~SimpleObject ();

	std::vector<Cell> children;
};

struct EnumObject : public SimpleObject
{
	EnumObject (Env::Function* ctor, size_t nchilds);
	virtual ~EnumObject ();

	Env::Function* ctor;
};


}}

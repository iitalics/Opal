#include "Env.h"
#include <map>
namespace Opal { namespace Env {
;

static std::map<int, Type*> _functions;
static std::map<int, Type*> _tuples;

static std::string specialName (const std::string& prefix, size_t n)
{
	std::ostringstream ss;
	ss << prefix << "(" << n << ")";
	return ss.str();
}

Type* Type::function (size_t argc)
{
	auto it = _functions.find(argc);
	if (it != _functions.end())
		return it->second;

	auto coreMod = Module::getCore();
	auto ty = new Type(
		"fn",
		coreMod,
		argc + 1,
		false);
	ty->_function = true;
	ty->data.fields = nullptr;
	ty->data.nfields = 0;
	_functions[argc] = ty;
	return ty;
}
Type* Type::tuple (size_t argc)
{
	auto it = _tuples.find(argc);
	if (it != _tuples.end())
		return it->second;

	auto coreMod = Module::getCore();
	auto ty = new Type(
		specialName("tuple", argc),
		coreMod,
		argc,
		false);
	ty->_tuple = true;
	ty->data.fields = nullptr;
	ty->data.nfields = 0;
	_tuples[argc] = ty;
	return ty;
}
Type* Type::core (const std::string& name)
{
	return Module::getCore()->getType(name);
}



static void makePrimitives ()
{
	auto coreMod = Module::getCore();

	auto intType = new Type("int", coreMod, 0, false);
	auto realType = new Type("real", coreMod, 0, false);
	auto boolType = new Type("bool", coreMod, 0, false);
	auto charType = new Type("char", coreMod, 0, false);
	auto stringType = new Type("string", coreMod, 0, false);
	auto unitType = new Type("unit", coreMod, 0, false);

	// TODO: GC info, default initializers
	intType->gc_collected = false;
	realType->gc_collected = false;
	boolType->gc_collected = false;
	charType->gc_collected = false;
	unitType->gc_collected = false;
	stringType->gc_collected = true;

	coreMod->types.insert(coreMod->types.begin(),
		{ intType,
		  realType,
		  boolType,
		  charType,
		  stringType,
		  unitType });
}
Module* Module::getCore ()
{
	static bool init = false;
	if (!init)
	{
		init = true;
		makePrimitives();
	}

	return get("Core");
}

}}

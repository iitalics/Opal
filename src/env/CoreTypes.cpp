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


static Module* _internal = nullptr;
static Module* getInternal ()
{
	if (!_internal)
		return (_internal = Module::get("(internal)"));
	else
		return _internal;
}


Type* Type::function (size_t argc)
{
	auto it = _functions.find(argc);
	if (it != _functions.end())
		return it->second;

	auto mod = getInternal();
	auto ty = new Type("fn", mod, argc + 1, false);
	ty->_function = true;
	ty->data.fields = nullptr;
	ty->data.nfields = 0;
	_functions[argc] = ty;
	mod->types.push_back(ty);
	return ty;
}
Type* Type::tuple (size_t argc)
{
	auto it = _tuples.find(argc);
	if (it != _tuples.end())
		return it->second;

	auto mod = getInternal();
	auto ty = new Type(specialName("tuple", argc), mod, argc, false);
	ty->_tuple = true;
	ty->data.nfields = 0;
	ty->data.fields = nullptr;
	_tuples[argc] = ty;
	mod->types.push_back(ty);
	return ty;
}
Type* Type::core (const std::string& name)
{
	return Module::getCore()->getType(name);
}
Type* Type::anonIFace (const std::string& name)
{
	auto mod = getInternal();
	auto ty = new Type("." + name, mod, 1, true);
	auto fnsigs = new IFaceSignature[1];

	// iface method
	ty->iface.nfuncs = 1;
	ty->iface.funcs = fnsigs;
	fnsigs[0].name = name;
	fnsigs[0].type = Infer::Type::param(1, "-");

	// env method (for runtime?)
	auto method = new Function(Function::IFaceFunction, name, mod);
	method->ifaceSig = &fnsigs[0];
	method->parent = ty;
	ty->methods.push_back(method);

	return ty;
}


static void makePrimitives ()
{
	auto coreMod = Module::getCore();

	auto intType = new Type("int", coreMod, 0, false);
	auto realType = new Type("real", coreMod, 0, false);
	auto longType = new Type("long", coreMod, 0, false);
	auto boolType = new Type("bool", coreMod, 0, false);
	auto charType = new Type("char", coreMod, 0, false);
	auto unitType = new Type("unit", coreMod, 0, false);
	auto boxType = new Type(".box", coreMod, 1, false);
	auto stringType = new Type("string", coreMod, 0, false);

	// TODO: GC info, default initializers
	intType->gc_collected = false;
	realType->gc_collected = false;
	longType->gc_collected = false;
	boolType->gc_collected = false;
	charType->gc_collected = false;
	unitType->gc_collected = false;
	boxType->gc_collected = true;
	stringType->gc_collected = true;

	coreMod->types.insert(coreMod->types.begin(),
		{ intType,
		  realType,
		  longType,
		  boolType,
		  charType,
		  unitType,
		  boxType,
		  stringType });
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

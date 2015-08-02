#include "Env.h"
#include <map>

namespace Opal { namespace Env {
;

Module* Module::_all = nullptr;

static int unnamed = 0;
static std::string unname ()
{
	std::ostringstream ss;
	ss << "@mod_" << (unnamed++);
	return ss.str();
}


Module::Module (const std::string& _name)
	: name(_name), _next(_all)
{
	_all = this;
}
Module::Module ()
	: Module(unname()) {}

Module::~Module ()
{
	if (_all == this)
		_all = _next;

	for (auto& ty : types)
		delete ty;
	for (auto& g : globals)
		delete g;
}




Module* Module::get (const std::string& name)
{
	for (auto mod = _all; mod != nullptr; mod = mod->_next)
		if (mod->name == name)
			return mod;

	return new Module(name);
}
Type* Module::getType (const std::string& name) const
{
	for (auto& ty : types)
		if (ty->name == name)
			return ty;

	return nullptr;
}
Global* Module::getGlobal (const std::string& name) const
{
	for (auto& g : globals)
		if (g->name == name)
			return g;

	return nullptr;
}



Type::Type (const std::string& _name,
		Module* _mod, size_t _nparams,
		bool _iface, const Span& _span)
	: name(_name), module(_mod), declSpan(_span), nparams(_nparams), isIFace(_iface)
{
	_function = _tuple = false;

	if (_iface)
	{
		iface.funcs = nullptr;
		iface.nfuncs = 0;
	}
	else
	{
		data.fields = nullptr;
		data.nfields = 0;
	}
}

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
		specialName("@fn", argc),
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
		specialName("@tuple", argc),
		coreMod,
		argc,
		false);
	ty->_tuple = true;
	ty->data.fields = nullptr;
	ty->data.nfields = 0;
	_tuples[argc] = ty;
	return ty;
}



AST::Name Type::fullname () const { return AST::Name(name, module->name); }
AST::Name Global::fullname () const { return AST::Name(name, module->name); }
AST::Name Function::fullname () const { return AST::Name(name, module->name); }



// deconstructors
void DataType::destroy ()
{
	delete[] fields;
}
void IFaceType::destroy ()
{
	delete[] funcs;
}
IFaceSignature::~IFaceSignature ()
{
	delete[] args;
}
Type::~Type ()
{
	if (isIFace)
		iface.destroy();
	else
		data.destroy();
}
Global::~Global ()
{
	if (func != nullptr)
		delete func;
}
Function::~Function () {}




}}

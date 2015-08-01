#include "Env.h"
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
Funcion::~Function () {}




}}

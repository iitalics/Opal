#include "Env.h"
namespace Opal { namespace Env {
;

Module* Module::_all = nullptr;
static int unnamed = 0;

Module::Module (const std::string& _name)
	: name(_name), _next(_all)
{}
Module::~Module ()
{
	// delete things or nah?
}

Module* Module::get (const std::string& name)
{
	for (auto mod = _all; mod != nullptr; mod = mod->_next)
		if (mod->name == name)
			return mod;

	return make(name);
}
Module* Module::make (const std::string& name)
{
	return (_all = new Module(name));
}
Module* Module::make ()
{
	std::ostringstream ss;
	ss << "@mod_" << (unnamed++);
	return make(ss.str());
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




// deconstructors
void DataType::destroy ()
{
	delete[] params;
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





}}

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
	for (auto& mod = _all; mod != nullptr; mod = mod->_next)
		if (mod->name == name)
			return mod;

	return make(name);
}
Module* Module::make (const std::string& name)
{
	auto mod = new Module(name);
	_all = mod;
	return mod;
}
Module* Module::make ()
{
	std::ostringstream ss;
	ss << "@mod_" << (unnamed++);
	return make(ss.str());
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

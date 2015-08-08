#include "Env.h"
#include "../infer/Analysis.h"
#include <map>
namespace Opal { namespace Env {
;

Module* Module::_all = nullptr;

static std::string _unname ()
{
	static int n = 0;
	std::ostringstream ss;
	ss << "@mod_" << (n++);
	return ss.str();
}


Module::Module (const std::string& _name)
	: name(_name), _next(_all)
{
	_all = this;
}
Module::Module ()
	: Module(_unname()) {}

Module::~Module ()
{
	if (_all == this)
		_all = _next;

	for (auto& ty : types)
		delete ty;
	for (auto& g : globals)
		delete g;
}


static std::map<std::string, Module*> _modules;

Module* Module::get (const std::string& name)
{
	auto it = _modules.find(name);
	if (it == _modules.end())
	{
		auto mod = new Module(name);
		_modules[name] = mod;
		return mod;
	}
	else
		return it->second;
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



Infer::TypePtr Global::getType ()
{
	if (isFunc)
		return func->getType();
	else
		return func->ret;
}


Function::Function (Kind _kind, const std::string& _name, Module* _mod,
		const Span& _span)
	: kind(_kind), name(_name), module(_mod), declSpan(_span)
{
	parent = nullptr;
	ret = nullptr;
	analysis = nullptr;
}
void Function::infer ()
{
	if (ret != nullptr)
		return;

	Infer::Analysis inferer(nm, args);
	analysis = &inferer;

	ret = inferer.ret;
	inferer.infer(body, ret);
	inferer.polyToParam(ret);

	analysis = nullptr;

	std::cout << fullname().str() << " -> " << ret->str() << std::endl;	
}
Infer::TypePtr Function::getType ()
{
	Infer::TypeList types(ret);
	size_t argc = 0;

	// add to linked list in reverse
	for (int i = args.size() - 1; ; i--)
	{
		if (i == 0 && parent != nullptr) // don't include "self" argument
			break;
		if (i < 0)
			break;

		types = Infer::TypeList(args[i].type, types);
		argc++;
	}

	return Infer::Type::concrete(Type::function(argc), types);
}
Infer::TypePtr IFaceSignature::getType ()
{
	Infer::TypeList types(ret);

	for (int i = argc; i-- > 0; )
		types = Infer::TypeList(args[i], types);
	
	return Infer::Type::concrete(Type::function(argc), types);
}


AST::Name Type::fullname () const { return AST::Name(name, module->name); }
AST::Name Global::fullname () const { return AST::Name(name, module->name); }
AST::Name Function::fullname () const
{
	if (parent == nullptr)
		return AST::Name(name, module->name);
	else
		return AST::Name(parent->name + "." + name, parent->module->name);
}



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

#include "Analysis.h"
#include "../env/Namespace.h"
namespace Opal { namespace Infer {
;


Analysis::Analysis (Env::Function* fn, Analysis* _call)
	: parent(fn), nm(fn->nm), _ctx(nm), _calledBy(_call), _finished(false)
{
	ret = parent->ret = Type::poly();

	depends = new Depends();
	depends->insert(this);

	// define arguments
	for (auto& arg : parent->args)
	{
		_ctx.locateParams(arg.type);
		let(arg.name, arg.type);
	}
	_ctx.allowNewTypes = false;
}
Analysis::~Analysis () {}


// dependencies for circular calls
void Analysis::dependOn (Analysis* other)
{
	auto dep = depends;

	// merge dependency sets
	if (dep != other->depends)
	{
		for (auto an : *dep)
		{
			other->depends->insert(an);
			an->depends = other->depends;
		}
		delete dep;
	}
}
void Analysis::finish ()
{
	_finished = true;
}
bool Analysis::allFinished () const
{
	for (auto an : *depends)
		if (!an->_finished)
			return false;
	return true;
}

// convert poly types into params
TypePtr Analysis::polyToParam (TypePtr type)
{
	std::map<TypeWeakList*, TypePtr> with;
	return polyToParam(type, with);
}
TypePtr Analysis::polyToParam (TypePtr type,
		std::map<TypeWeakList*, TypePtr>& with)
{
	auto newArgs = type->args.map<TypePtr>([&] (TypePtr ty2)
	{
		return polyToParam(ty2, with);
	});

	if (type->kind == Type::Poly)
	{
		auto it = with.find(type->links);
		if (it == with.end())
		{
			std::ostringstream ss;
			ss << "." << _ctx.params.size();
			return
				with[type->links] = _ctx.createParam(ss.str(), newArgs);
		}
		else
			return it->second;
	}
	else if (newArgs.nil())
	{
		return type;
	}
	else if (type->kind == Type::Concrete)
	{
		return Type::concrete(type->base, newArgs);
	}
	else if (type->kind == Type::Param)
	{
		return Type::param(type->id, type->paramName, newArgs);
	}
	else
		return type;
}


// local scope variables
int Analysis::get (const std::string& name) const
{
	for (int i = stack.size(); i-- > 0;)
	{
		auto id = stack[i];
		if (allVars[id].name == name)
			return id;
	}
	return -1;
}
int Analysis::let (const std::string& name, TypePtr type)
{
	int id = allVars.size();
	allVars.push_back({
		id,
		name,
		type,
		false
	});
	stack.push_back(id);
	return id;
}


// infer a different function
TypePtr Analysis::_getFuncType (Env::Function* func)
{
	if (func->ret == nullptr)
		func->infer(this);
	else if (func->kind == Env::Function::CodeFunction &&
			func->analysis != nullptr)
	{
		// >> circular call <<

		// descend call stack and make everything dependent
		//  on us
		auto top = _calledBy;
		for (; top != nullptr; top = top->_calledBy)
		{
			top->dependOn(this);
			if (top == func->analysis)
				break;
		}
	}

	return func->getType();
}

// replace params from methods or fields e.g.
/*
	type T[#a] { x : list[#a] }
	...
	let bar : T[int]
	bar.x  ->  list[int]
*/
TypePtr Analysis::_inst (TypePtr obj, TypePtr type, TypePtr self)
{
	std::vector<TypePtr> with;

	// first "argument" of ifaces is itself e.g.
	/*
		iface #t : Ord { fn cmp (#t) : int }
		      ^                  ^
	*/
	if (self != nullptr)
		with.push_back(self);

	for (auto arg : obj->args)
		with.push_back(arg);

	return replaceParams(type, with);
}
TypePtr Analysis::_instMethod (TypePtr obj, Env::Function* fn)
{
	std::vector<TypePtr> with;

	auto fn_self = replaceParams(fn->args[0].type, with);

	/*
	_unify() fails if the 'obj' type and the 'impl' type
	 are incompatible e.g.

		type T[#a] { ... }
		impl T[int] {
			fn foo () { ... }
		}
		fn bar (t : T[string]) {
			t.foo
			  ^ missing field 'foo'
		}
	*/
	if (_unify(fn_self, obj) != UnifyOK)
		return nullptr;

	return replaceParams(_getFuncType(fn), with);
}
TypePtr Analysis::_findField (TypePtr obj, const std::string& name, int& out)
{
	auto base = obj->base;

	for (size_t i = 0; i < base->data.nfields; i++)
		if (base->data.fields[i].name == name)
		{
			out = int(i);
			return _inst(obj, base->data.fields[i].type);
		}

	return nullptr;
}
TypePtr Analysis::_findMethod (TypePtr obj, const std::string& name, Env::Function*& out)
{
	// look for defined methods
	if (obj->kind == Type::Concrete)
	{
		if (obj->base->isIFace)
			return _findIFaceFunc(obj, obj, name, out);

		for (auto fn : obj->base->methods)
			if (fn->name == name)
			{
				auto res = _instMethod(obj, fn);
				if (res)
				{
					out = fn;
					return res;
				}
				else // failed!
					return nullptr;
			}

		return nullptr;
	}

	// look for iface functions in each iface
	for (auto iface : obj->args)
	{
		auto res = _findIFaceFunc(obj, iface, name, out);
		if (res)
			return res;
	}
	return nullptr;
}
TypePtr Analysis::_findIFaceFunc (TypePtr obj, TypePtr iface, const std::string& name, Env::Function*& out)
{
	auto base = iface->base;

	for (size_t i = 0; i < base->iface.nfuncs; i++)
		if (base->iface.funcs[i].name == name)
		{
			out = nullptr; // TOOD: FIX THIS
			return _inst(obj, base->iface.funcs[i].getType(), obj);
		}

	return nullptr;
}


}}

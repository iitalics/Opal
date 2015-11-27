#include "Analysis.h"
#include "../env/Namespace.h"
namespace Opal { namespace Infer {
;


Analysis::Analysis (Env::Function* fn, Analysis* _call)
	: parent(fn), nm(fn->nm), _ctx(nm)
	, _calledBy(_call), _finished(false), _temps(0)
{
	ret = parent->ret = Type::poly();
	env = parent->localEnv = new LocalEnv();

	depends = new Depends();
	depends->insert(this);

	// define arguments
	for (auto& arg : parent->args)
	{
		_ctx.locateParams(arg.type);
		let(arg.name, arg.type);
	}
//	_ctx.allowNewTypes = false;
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
bool Analysis::allFinished () const
{
	for (auto an : *depends)
		if (!an->_finished)
			return false;
	return true;
}
void Analysis::finish ()
{
	_finished = true;
}

// convert param types into polys
TypePtr Analysis::replaceParams (TypePtr ty, std::vector<TypePtr>& with)
{
	if (ty->kind == Type::Param)
	{
		size_t idx = ty->id;

		while (with.size() <= idx)
			with.push_back(nullptr);

		if (with[idx] == nullptr)
		{
			auto res = Type::poly();
			with[idx] = res;
			res->args = ty->args.map<TypePtr>([&] (TypePtr ty2) {
				return replaceParams(ty2, with);
			});
			return res;
		}
		else
			return with[idx];
	}
	else if (ty->kind == Type::Concrete && !ty->args.nil())
	{
		auto newArgs = ty->args.map<TypePtr>([&] (TypePtr ty2) {
			return replaceParams(ty2, with);
		});
		return Type::concrete(ty->base, newArgs);
	}
	else
		return ty;
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
	if (type->kind == Type::Poly)
	{
		auto it = with.find(type->links);
		if (it == with.end())
		{
			// TODO: pretty names
			std::ostringstream ss;
			ss << "." << _ctx.params.size();

			auto param = _ctx.createParam(ss.str(), TypeList());
			with[type->links] = param;
			param->args = type->args.map<TypePtr>([&] (TypePtr ty2) { return polyToParam(ty2, with); });
			return param;
		}
		else
			return it->second;
	}
	else if (type->args.nil())
		return type;

	auto newArgs = type->args.map<TypePtr>([&] (TypePtr ty2) { return polyToParam(ty2, with); });

	if (type->kind == Type::Concrete)
		return Type::concrete(type->base, newArgs);
	else if (type->kind == Type::Param)
		return Type::param(type->id, type->paramName, newArgs);
	else
		return type;
}
void Analysis::polyToParamArgs ()
{
	std::map<TypeWeakList*, TypePtr> with;

	for (auto& arg : parent->args)
		arg.type = polyToParam(arg.type, with);
	parent->ret = polyToParam(parent->ret, with);
}


// local scope variables
LocalVar* Analysis::get (const std::string& name) const
{
	for (size_t i = 0, len = stack.size(); i < len; i++)
	{
		auto var = stack[len - i - 1];
		if (var->name == name)
		{
			// possibly set ref flag
			env->ref(var);
			return var;
		}
	}
	return nullptr;
}
LocalVar* Analysis::let (const std::string& name, TypePtr type)
{
	auto var = env->define(name, type);
	stack.push_back(var);
	return var;
}
LocalVar* Analysis::temp (TypePtr type)
{
	std::ostringstream ss;
	ss << "~" << (_temps++);
	return let(ss.str(), type);
}

size_t Analysis::stackSave ()
{
	return stack.size();
}
void Analysis::stackRestore (size_t n)
{
	stack.resize(n);
}



// LocalEnv
LocalEnv::LocalEnv ()
	: containing(nullptr) {}
LocalEnv::~LocalEnv ()
{
	for (auto var : defs)
		delete var;
}
LocalVar* LocalEnv::define (const std::string& name, TypePtr ty)
{
	auto var = new LocalVar {
		.parent = this,
		.name = name, .type = ty,
		.didMut = false, .didRef = false
	};
	defs.push_back(var);
	return var;
}
void LocalEnv::ref (LocalVar* var)
{
	if (var->parent == this) return;

	for (auto var2 : refs)
		if (var2 == var)
			return;
	refs.push_back(var);
	var->didRef = true;

	if (containing != nullptr)
		containing->ref(var);
}
size_t LocalEnv::index (LocalVar* var) const
{
	// refs first, then defs. so that currying works
	size_t nrefs = refs.size();
	size_t ndefs = defs.size();

	for (size_t i = 0; i < nrefs; i++)
		if (refs[i] == var)
			return i;
	for (size_t i = 0 ; i < ndefs; i++)
		if (defs[i] == var)
			return i + nrefs;
	return -1;
}
size_t LocalEnv::size () const
{
	return refs.size() + defs.size();
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
		while (top != nullptr)
		{
			top->dependOn(this);
			if (top == func->analysis)
				break;
			else
				top = top->_calledBy;
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
TypePtr Analysis::_findProperty (TypePtr obj, const std::string& name, int& out)
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

		if (auto fn = obj->base->getMethod(name))
			if (auto res = _instMethod(obj, fn))
			{
				out = fn;
				return res;
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

	if (obj->kind == Type::Poly)
	{
		// create anonymous iface
		auto ty = Type::poly();
		auto iface = Env::Type::anonIFace(name);
		auto ifaceTy = Type::concrete(iface, { ty });
		auto withPoly = Type::poly(TypeList(ifaceTy));
		if (_unify(obj, withPoly) != UnifyOK) // this SHOULD always be ok
			return nullptr;

		out = iface->methods[0];
		return ty;
	}
	else
		return nullptr;
}
TypePtr Analysis::_findIFaceFunc (TypePtr obj, TypePtr iface, const std::string& name, Env::Function*& out)
{
	auto base = iface->base;

	for (size_t i = 0; i < base->iface.nfuncs; i++)
		if (base->iface.funcs[i].name == name)
		{
			out = base->methods[i];
			return _inst(iface, base->iface.funcs[i].type, obj);
		}

	return nullptr;
}


}}

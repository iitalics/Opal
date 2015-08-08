#include "Analysis.h"
#include "../env/Namespace.h"
namespace Opal { namespace Infer {
;


Analysis::Analysis (Env::Namespace* _nm,
		const std::vector<Var>& args)
	: nm(_nm), _ctx(_nm)
{
	// define arguments
	for (auto& arg : args)
	{
		_ctx.locateParams(arg.type);
		let(arg.name, arg.type);
	}
}
Analysis::~Analysis () {}


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

void Analysis::infer (AST::ExpPtr e, TypePtr dest)
{
	static auto stringType = Type::concrete(Env::Type::core("string"), TypeList());
	static auto realType = Type::concrete(Env::Type::core("real"), TypeList());
	static auto boolType = Type::concrete(Env::Type::core("bool"), TypeList());

	// TODO: type inference HERE
	if (dynamic_cast<AST::StringExp*>(e.get()))
		unify(dest, stringType, e->span);
	if (dynamic_cast<AST::RealExp*>(e.get()))
		unify(dest, realType, e->span);
	if (dynamic_cast<AST::BoolExp*>(e.get()))
		unify(dest, boolType, e->span);
	else if (auto e2 = dynamic_cast<AST::VarExp*>(e.get()))
		_infer(e2, dest);
	else if (auto e2 = dynamic_cast<AST::IntExp*>(e.get()))
		_infer(e2, dest);
	else if (auto e2 = dynamic_cast<AST::FieldExp*>(e.get()))
		_infer(e2, dest);
	else if (auto e2 = dynamic_cast<AST::CallExp*>(e.get()))
		_infer(e2, dest);
	else if (auto e2 = dynamic_cast<AST::BlockExp*>(e.get()))
		_infer(e2, dest);
	else if (auto e2 = dynamic_cast<AST::TupleExp*>(e.get()))
		_infer(e2, dest);
	else if (auto e2 = dynamic_cast<AST::CondExp*>(e.get()))
		_infer(e2, dest);
}
void Analysis::_infer (AST::VarExp* e, TypePtr dest)
{
	Env::Global* global;
	int id;
	TypePtr type;

	auto name = e->name;
	if (name.hasModule() || (id = get(name.name)) == -1)
	{
		global = nm->getGlobal(name); // get global from namespace

		if (global == nullptr)
		{
			std::ostringstream ss;
			ss << "undefined global '" << name.str() << "'";
			throw SourceError(ss.str(), e->span);
		}

		e->global = global;
		type = global->getType();

		// turn params into poly
		std::vector<TypePtr> repl;
		type = replaceParams(type, repl);
	}
	else
	{
		e->varId = id;
		type = allVars[id].type;
	}

	unify(dest, type, e->span);
}
void Analysis::_infer (AST::IntExp* e, TypePtr dest)
{
	static auto core_int = Env::Type::core("int");
	static auto core_real = Env::Type::core("real");
	static auto core_long = Env::Type::core("long");
	static auto intType = Type::concrete(core_int, TypeList());

	// implicitly casts to real/long if necessary
	if (dest->kind == Type::Concrete)
	{
		if (dest->base == core_real)
		{
			e->castReal = true;
			return;
		}
		else if (dest->base == core_long)
		{
			e->castLong = true;
			return;
		}
		else if (dest->base == core_int)
			return;
	}

	// defaults to Core::int
	unify(dest, intType, e->span);
}


TypePtr Analysis::_getFieldType (int& idx_out, Env::Type* base, const std::string& name)
{
	for (size_t i = 0; i < base->data.nfields; i++)
		if (base->data.fields[i].name == name)
		{
			idx_out = i;
			return base->data.fields[i].type;
		}
	return nullptr;
}
TypePtr Analysis::_getIFaceFuncType (Env::Type* base, const std::string& name)
{
	for (size_t i = 0; i < base->iface.nfuncs; i++)
		if (base->iface.funcs[i].name == name)
		{
			// TODO: get a function?
			return base->iface.funcs[i].getType();
		}
	return nullptr;
}
TypePtr Analysis::_getMethodType (Env::Function*& fnout, Env::Type* base, const std::string& name)
{
	for (auto fn : base->methods)
		if (fn->name == name)
		{
			fnout = fn;
			return fn->getType();
		}
	return nullptr;
}
TypePtr Analysis::_instField (TypePtr self, TypePtr type)
{
	std::vector<TypePtr> with;

	for (auto arg : self->args)
		with.push_back(arg);

	return replaceParams(type, with);
}
TypePtr Analysis::_instIFace (TypePtr self, TypePtr type)
{
	std::vector<TypePtr> with;
	with.push_back(self);

	for (auto arg : self->args)
		with.push_back(arg);

	return replaceParams(type, with);
}
TypePtr Analysis::_instMethod (TypePtr self, TypePtr type, Env::Function* fn)
{
	std::vector<TypePtr> with;

	auto fn_self = fn->args[0].type;
	fn_self = replaceParams(fn_self, with);

	// not actually of type
	/*  e.g.
		type A[#t] {...}
		impl A[int] {
			fn foo () {}
		}
		fn bar (a : A[string]) {
			a.foo()
			  ^ error missing field 'foo'
		}
	*/
	if (_unify(self, fn_self) != UnifyOK)
		return nullptr;

	return replaceParams(type, with);
}


void Analysis::_infer (AST::FieldExp* e, TypePtr dest)
{
	auto objType = newType();
	infer(e->children[0], objType);
	TypePtr res = nullptr;

	// save this now in case unify() overwrites some things
	//  and then fails
	auto typeName = objType->str();

	if (objType->kind == Type::Concrete)
	{
		auto base = objType->base;

		if (base->isIFace)
		{
			// get method of iface type
			if ((res = _getIFaceFuncType(base, e->name)))
				res = _instIFace(objType, res);
		}
		else
		{
			int index;

			// or get field of normal type
			if ((res = _getFieldType(index, base, e->name)))
			{
				res = _instField(objType, res);
				e->index = index;
			}
		}

		if (res == nullptr)
		{
			Env::Function* fn;
			// or get a method
			if ((res = _getMethodType(fn, base, e->name)))
			{
				res = _instMethod(objType, res, fn);
				e->method = fn;
			}
		}
	}
	else // Poly / Param
	{
		// get method of iface
		for (auto iface : objType->args)
			if ((res = _getIFaceFuncType(iface->base, e->name)))
			{
				res = _instIFace(objType, res);
				break;
			}
	}

	// no luck.
	if (res == nullptr)
	{
		std::ostringstream ss;
		ss << "type is missing field '" << e->name << "'";
		throw SourceError(ss.str(),
			{ "type: " + typeName }, e->span);
	}

	unify(dest, res, e->span);
	return;
}
void Analysis::_infer (AST::CallExp* e, TypePtr dest)
{
	/*
		kind of weird algorithm.  get the number of arguments,
	     get the type of the function to be called, then
	     make a "model" type to represent the function.
	     unify the model and the function type, then infer
	     each argument against the types in each argument of the
	     model type.
		e.g.

		fn repeat (msg : string, n : int) : unit
		
		infer({ repeat("hi", 5) }, dest)
			infer({ repeat }, fnty)
			fnty := fn(string, int) -> unit
			fnmodel := fn(_1, _2) -> _3

			unify(fnmodel, fnty)
			_1 <- string
			_2 <- int
			_3 <- unit

			infer({ "hi" }, _1 = string) // Ok
			infer({ 5 }, _2 = int)       // Ok

			unify(dest, _3 = unit)       // Ok

		{ repeat("hi", 5) } : unit
	*/
	auto fnty = newType();
	infer(e->children[0], fnty);

	if (fnty->kind == Type::Concrete)
	{
		if (!fnty->base->isFunction())
			throw SourceError("attempt to call type '" + fnty->str() + "'",
					e->span);

		if (e->children.size() != fnty->args.size())
		{
			std::ostringstream ss1, ss2;
			ss1 << "expected: " << fnty->args.size() - 1;
			ss2 << "found: " << e->children.size() - 1;

			throw SourceError("wrong number of arguments to function",
					{ ss1.str(), ss2.str() }, e->span);
		}
	}

	// create model for function based on # arguments
	std::vector<TypePtr> args;
	size_t argc = e->children.size() - 1;
	auto ret = newType();

	args.reserve(argc);
	for (size_t i = 0; i < argc; i++)
		args.push_back(newType());
	args.push_back(ret);

	auto fnmodel = Type::concrete(Env::Type::function(argc), TypeList(args));

	// get argument types
	unify(fnmodel, fnty, e->span);

	// infer arguments
	for (size_t i = 0; i < argc; i++)
		infer(e->children[i + 1], args[i]);
	
	// push return value
	unify(dest, ret, e->span);
}
void Analysis::_infer (AST::BlockExp* e, TypePtr dest)
{
	static auto unitType = Type::concrete(Env::Type::core("unit"), TypeList());

	auto res = unitType;
	for (auto& e2 : e->children)
	{
		res = newType();
		infer(e2, res);
	}
	if (e->unitResult)
		res = unitType;

	unify(dest, res, e->span);
}
void Analysis::_infer (AST::TupleExp* e, TypePtr dest)
{
	static auto unitType = Type::concrete(Env::Type::core("unit"), TypeList());

	if (e->children.empty())
	{
		unify(dest, unitType, e->span);
		return;
	}

	// similar to the algorithm for AST::CallExp
	std::vector<TypePtr> args;
	size_t nvals = e->children.size();
	args.reserve(nvals);
	for (auto e2 : e->children)
		args.push_back(newType());

	auto model = Type::concrete(Env::Type::tuple(nvals), TypeList(args));
	unify(model, dest, e->span);

	for (size_t i = 0; i < nvals; i++)
		infer(e->children[i], args[i]);
}
void Analysis::_infer (AST::CondExp* e, TypePtr dest)
{
	static auto boolType = Type::concrete(Env::Type::core("bool"), TypeList());

	auto res = newType();

	infer(e->children[0], boolType);
	infer(e->children[1], res);
	if (e->children.size() > 2)
		infer(e->children[2], res);

	unify(dest, res, e->span);
}


}}

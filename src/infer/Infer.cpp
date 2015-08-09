#include "Analysis.h"
#include "../env/Namespace.h"
namespace Opal { namespace Infer {
;


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
		_getFuncType(global->func);
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

void Analysis::_infer (AST::FieldExp* e, TypePtr dest)
{
	TypePtr res = nullptr;
	auto obj = Type::poly();
	infer(e->children[0], obj);

	// save this now in case unify() overwrites some things
	//  and then fails
	auto typeName = obj->str();

	// look for fields
	if (obj->kind == Type::Concrete &&
			!obj->base->isIFace)
	{
		int index;
		if ((res = _findField(obj, e->name, index)))
		{
			e->index = index;
			e->method = nullptr;
		}
	}

	// look for methods
	if (res == nullptr)
	{
		Env::Function* fn;
		if ((res = _findMethod(obj, e->name, fn)))
			e->method = fn;
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
	auto fnty = Type::poly();
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
	auto ret = Type::poly();

	args.reserve(argc);
	for (size_t i = 0; i < argc; i++)
		args.push_back(Type::poly());
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

	bool returnUnit = e->unitResult || e->children.empty();

	size_t ignored = e->children.size();
	if (!returnUnit)
		ignored--;

	for (size_t i = 0; i < ignored; i++)
		infer(e->children[i], Type::poly());

	if (returnUnit)
		unify(dest, unitType, e->span);
	else
		infer(e->children.back(), dest);
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
		args.push_back(Type::poly());

	auto model = Type::concrete(Env::Type::tuple(nvals), TypeList(args));
	unify(dest, model, e->span);

	for (size_t i = 0; i < nvals; i++)
		infer(e->children[i], args[i]);
}

void Analysis::_infer (AST::CondExp* e, TypePtr dest)
{
	static auto boolType = Type::concrete(Env::Type::core("bool"), TypeList());

	auto res = Type::poly();

	infer(e->children[0], boolType);
	infer(e->children[1], res);
	if (e->children.size() > 2)
		infer(e->children[2], res);

	unify(dest, res, e->span);
}

}}
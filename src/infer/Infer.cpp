#include "Analysis.h"
#include "../env/Namespace.h"
namespace Opal { namespace Infer {
;

// cache core types required by the inferer
static Env::Type* core_unit, *core_int, *core_bool, *core_real, *core_string;
static TypePtr unitType, intType, boolType, realType, stringType;
void Analysis::initTypes ()
{
	core_unit = Env::Type::core("unit");
	core_int = Env::Type::core("int");
	core_bool = Env::Type::core("bool");
	core_real = Env::Type::core("real");
	core_string = Env::Type::core("string");

	unitType = Type::concrete(core_unit, TypeList());
	intType = Type::concrete(core_int, TypeList());
	boolType = Type::concrete(core_bool, TypeList());
	realType = Type::concrete(core_real, TypeList());
	stringType = Type::concrete(core_string, TypeList());
}



void Analysis::infer (AST::ExpPtr e, TypePtr dest)
{
	if (dynamic_cast<AST::StringExp*>(e.get()))
		unify(dest, stringType, e->span);
	if (dynamic_cast<AST::RealExp*>(e.get()))
		unify(dest, realType, e->span);
	if (dynamic_cast<AST::BoolExp*>(e.get()))
		unify(dest, boolType, e->span);
	if (dynamic_cast<AST::GotoExp*>(e.get()))
		; // assume this theoretically returns the right thing always
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
	else if (auto e2 = dynamic_cast<AST::LazyOpExp*>(e.get()))
		_infer(e2, dest);
	else if (auto e2 = dynamic_cast<AST::CompareExp*>(e.get()))
		_infer(e2, dest);
	else if (auto e2 = dynamic_cast<AST::ObjectExp*>(e.get()))
		_infer(e2, dest);
	else if (auto e2 = dynamic_cast<AST::ReturnExp*>(e.get()))
		_infer(e2);
	else if (auto e2 = dynamic_cast<AST::LetExp*>(e.get()))
		_infer(e2);
	else if (auto e2 = dynamic_cast<AST::AssignExp*>(e.get()))
		_infer(e2);
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
	// implicitly casts to real/long if necessary
	if (dest->kind == Type::Concrete)
	{
		if (dest->base == core_real)
		{
			e->castReal = true;
			return;
		}
/*		else if (dest->base == core_long)
		{
			e->castLong = true;
			return;
		}*/
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
		ss << "undefined field '" << e->name << "'";
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
			fnmodel := fn(_1, _2) -> dest

			unify(fnmodel, fnty)
			_1 <- string
			_2 <- int
			dest <- unit

			infer({ "hi" }, _1 = string) // Ok
			infer({ 5 }, _2 = int)       // Ok

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

		TypePtr given_ret;
		for (auto xs = fnty->args; !xs.nil(); ++xs)
			given_ret = xs.head();

		// unifying here gives prettier error messages
		//  in case of type mismatch
		if (given_ret)
			unify(dest, given_ret, e->span);
	}

	// create model for function based on # arguments
	std::vector<TypePtr> args;
	size_t argc = e->children.size() - 1;

	args.reserve(argc);
	for (size_t i = 0; i < argc; i++)
		args.push_back(Type::poly());
	args.push_back(dest); // return value dest

	// get argument types
	auto fnmodel = Type::concrete(Env::Type::function(argc), TypeList(args));
	unify(fnmodel, fnty, e->span);

	// infer arguments
	for (size_t i = 0; i < argc; i++)
		infer(e->children[i + 1], args[i]);


	// find out if we're calling a static function
	auto fne = e->children[0];
	if (auto field = dynamic_cast<AST::FieldExp*>(fne.get()))
	{
		e->function = field->method;
	}
	else if (auto var = dynamic_cast<AST::VarExp*>(fne.get()))
	{
		auto global = var->global;
		if (global != nullptr && global->isFunc)
			e->function = global->func;
	}
}

void Analysis::_infer (AST::BlockExp* e, TypePtr dest)
{
	bool returnUnit = e->unitResult || e->children.empty();
	size_t nstack = stack.size();

	// # of expressions to ignore
	size_t ignored = e->children.size();
	if (!returnUnit)
		ignored--;

	// infer stuff but ignore the result
	for (size_t i = 0; i < ignored; i++)
		infer(e->children[i], Type::poly());

	// either return unit or return the last expression
	if (returnUnit)
		unify(dest, unitType, e->span);
	else
		infer(e->children.back(), dest);

	// revert stack to original size
	stack.resize(nstack);
}

void Analysis::_infer (AST::TupleExp* e, TypePtr dest)
{
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
	auto res = Type::poly();

	infer(e->children[0], boolType);
	infer(e->children[1], res);
	if (e->children.size() > 2)
		infer(e->children[2], res);

	unify(dest, res, e->span);
}

void Analysis::_infer (AST::LazyOpExp* e, TypePtr dest)
{
	// expects 'bool' and returns 'bool'
	infer(e->children[0], boolType);
	infer(e->children[1], boolType);
	unify(dest, boolType, e->span);
}

void Analysis::_infer (AST::CompareExp* e, TypePtr dest)
{
	TypePtr desired;
	switch (e->kind)
	{
	// iface #t : Eq { fn equal (#t) -> bool }
	case AST::CompareExp::Eq:
	case AST::CompareExp::NotEq:
		desired = boolType;
		break;

	// iface #t : Ord { fn cmp (#t) -> int }
	default:
		desired = intType;
		break;
	}

	// expects a certain output type
	infer(e->children[0], desired);
	// ends up returning bool
	unify(dest, boolType, e->span);
}

void Analysis::_infer (AST::ObjectExp* e, TypePtr dest)
{
	auto type = Type::fromAST(e->objType, _ctx);

	if (type->kind != Type::Concrete || !type->base->userCreate)
		throw SourceError("cannot explicitly create instances of this type",
			e->objType->span);

	for (size_t i = 0, len = e->inits.size(); i < len; i++)
	{
		auto name = e->inits[i];

		for (size_t j = 0; j < i; j++)
			if (e->inits[j] == name)
			{
				std::ostringstream ss;
				ss << "field '" << name << "' initialized multiple times";
				throw SourceError(ss.str(), e->span);
			}

		int index;
		auto fieldType = _findField(type, name, index);
		if (fieldType == nullptr)
		{
			std::ostringstream ss;
			ss << "undefined field '" << name << "'";
			throw SourceError(ss.str(), e->span);
		}

		e->index.push_back(index);
		infer(e->children[i], fieldType);
	}

	if (e->inits.size() < type->base->data.nfields)
	{
		for (size_t i = 0, nfields = type->base->data.nfields; i < nfields; i++)
		{
			bool found = false;
			for (auto idx : e->index)
				if (idx == i)
				{
					found = true;
					break;
				}

			if (!found)
			{
				std::ostringstream ss;
				ss << "field '"
				   << type->base->data.fields[i].name
				   << "' requires initialization";
				throw SourceError(ss.str(), e->span);
			}
		}
	}

	e->base = type->base;
	unify(dest, type, e->span);
}

void Analysis::_infer (AST::ReturnExp* e)
{
	// infer returned value with expected return value of function
	infer(e->children[0], ret);
}

void Analysis::_infer (AST::LetExp* e)
{
	TypePtr type = Type::poly();
	infer(e->children[0], type);

	e->varId = let(e->name, type);
}

void Analysis::_infer (AST::AssignExp* e)
{
	// easy
	auto type = Type::poly();
	infer(e->children[0], type);
	infer(e->children[1], type);
}


}}

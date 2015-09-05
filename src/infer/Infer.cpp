#include "Analysis.h"
#include "../env/Namespace.h"
#include "../Names.h"
namespace Opal { namespace Infer {
;

// cache core types required by the inferer
static Env::Type* core_unit, *core_int, *core_bool, *core_char,
                 *core_real, *core_long, *core_string;
static TypePtr unitType, intType, boolType, charType, realType, longType, stringType;
void Analysis::initTypes ()
{
	core_unit = Env::Type::core("unit");
	core_int = Env::Type::core("int");
	core_bool = Env::Type::core("bool");
	core_char = Env::Type::core("char");
	core_real = Env::Type::core("real");
	core_long = Env::Type::core("long");
	core_string = Env::Type::core("string");

	unitType = Type::concrete(core_unit, TypeList());
	intType = Type::concrete(core_int, TypeList());
	boolType = Type::concrete(core_bool, TypeList());
	charType = Type::concrete(core_char, TypeList());
	realType = Type::concrete(core_real, TypeList());
	longType = Type::concrete(core_long, TypeList());
	stringType = Type::concrete(core_string, TypeList());
}



void Analysis::infer (AST::ExpPtr e, TypePtr dest)
{
	if (dynamic_cast<AST::StringExp*>(e.get()))
		unify(dest, stringType, e->span);
	else if (dynamic_cast<AST::BoolExp*>(e.get()))
		unify(dest, boolType, e->span);
	else if (dynamic_cast<AST::CharExp*>(e.get()))
		unify(dest, charType, e->span);
	else if (dynamic_cast<AST::GotoExp*>(e.get()))
		; // assume this theoretically returns the right thing always
	else if (auto e2 = dynamic_cast<AST::VarExp*>(e.get())) _infer(e2, dest);
	else if (auto e2 = dynamic_cast<AST::NumberExp*>(e.get())) _infer(e2, dest);
	else if (auto e2 = dynamic_cast<AST::FieldExp*>(e.get())) _infer(e2, dest);
	else if (auto e2 = dynamic_cast<AST::CallExp*>(e.get())) _infer(e2, dest);
	else if (auto e2 = dynamic_cast<AST::TypeHintExp*>(e.get())) _infer(e2, dest);
	else if (auto e2 = dynamic_cast<AST::BlockExp*>(e.get())) _infer(e2, dest);
	else if (auto e2 = dynamic_cast<AST::TupleExp*>(e.get())) _infer(e2, dest);
	else if (auto e2 = dynamic_cast<AST::CondExp*>(e.get())) _infer(e2, dest);
	else if (auto e2 = dynamic_cast<AST::LazyOpExp*>(e.get())) _infer(e2, dest);
	else if (auto e2 = dynamic_cast<AST::CompareExp*>(e.get())) _infer(e2, dest);
	else if (auto e2 = dynamic_cast<AST::ObjectExp*>(e.get())) _infer(e2, dest);
	else if (auto e2 = dynamic_cast<AST::LambdaExp*>(e.get())) _infer(e2, dest);
	else if (auto e2 = dynamic_cast<AST::MethodExp*>(e.get())) _infer(e2, dest);
	else if (auto e2 = dynamic_cast<AST::MatchExp*>(e.get())) _infer(e2, dest);
	else if (auto e2 = dynamic_cast<AST::ReturnExp*>(e.get())) _infer(e2);
	else if (auto e2 = dynamic_cast<AST::LetExp*>(e.get())) _infer(e2);
	else if (auto e2 = dynamic_cast<AST::AssignExp*>(e.get())) _infer(e2);
}

void Analysis::infer (AST::PatPtr p, TypePtr dest)
{
	if (auto p2 = dynamic_cast<AST::ConstPat*>(p.get())) _infer(p2, dest);
	else if (auto p2 = dynamic_cast<AST::BindPat*>(p.get())) _infer(p2, dest);
	else if (auto p2 = dynamic_cast<AST::EnumPat*>(p.get())) _infer(p2, dest);
}

void Analysis::_infer (AST::VarExp* e, TypePtr dest)
{
	LocalVar* var;
	TypePtr type;

	auto name = e->name;
	if (name.hasModule() || (var = get(name.name)) == nullptr)
	{
		auto global = nm->getGlobal(name); // get global from namespace

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
		e->var = var;
		type = var->type;
	}

	unify(dest, type, e->span);
}

void Analysis::_infer (AST::NumberExp* e, TypePtr dest)
{
	// int literals implicitly casts to real or long
	if (e->kind == AST::NumberExp::Int && dest->kind == Type::Concrete)
	{
		if (dest->base == core_real)
		{
			e->castReal();
			return;
		}
		else if (dest->base == core_long)
		{
			e->castLong();
			return;
		}
	}
	switch (e->kind)
	{
	case AST::NumberExp::Int:
		unify(dest, intType, e->span);
		break;
	case AST::NumberExp::Real:
		unify(dest, realType, e->span);
		break;
	case AST::NumberExp::Long:
		unify(dest, longType, e->span);
		break;
	default: break;
	}
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

void Analysis::_infer (AST::TypeHintExp* e, TypePtr dest)
{
	auto type = Type::fromAST(e->type, _ctx);

	// infer using given type instead of 'dest' type
	infer(e->children[0], type);
	unify(dest, type, e->span);
}

void Analysis::_infer (AST::BlockExp* e, TypePtr dest)
{
	bool returnUnit = e->unitResult || e->children.empty();
	size_t old = stackSave();

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

	stackRestore(old);
}

void Analysis::_infer (AST::TupleExp* e, TypePtr dest)
{
	if (e->children.empty())
	{
		unify(dest, unitType, e->span);
		return;
	}

	// similar to the algorithm for AST::CallExp
	TypeList args;
	size_t nargs = e->children.size();

	for (size_t i = 0; i < nargs; i++)
		args = TypeList(Type::poly(), args);

	auto model = Type::concrete(Env::Type::tuple(nargs), args);
	unify(dest, model, e->span);

	for (size_t i = 0; i < nargs; ++args, i++)
		infer(e->children[i], args.head());
}

void Analysis::_infer (AST::CondExp* e, TypePtr dest)
{
	infer(e->children[0], boolType);
	infer(e->children[1], dest);
	if (e->children.size() > 2)
		infer(e->children[2], dest);
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

void Analysis::_infer (AST::LambdaExp* e, TypePtr dest)
{
	auto ret = Type::poly();
	auto args = TypeList(ret);

	for (size_t i = 0, len = e->args.size(); i < len; i++)
	{
		auto arg = e->args[len - i - 1];
		TypePtr argType;

		if (arg.type == nullptr)
			argType = Type::poly();
		else
			argType = Type::fromAST(arg.type, _ctx);

		args = TypeList(argType, args);
	}

	auto model = Type::concrete(
		Env::Type::function(e->args.size()),
		args);

	// unify with model
	unify(dest, model, e->span);

	// save old environment
	size_t nstack = stack.size();
	auto oldEnv = env;

	// create new environemnt
	env = e->env = new LocalEnv();
	for (size_t i = 0, len = e->args.size(); i < len; ++args, i++)
		let(e->args[i].name, args.head());

	// infer body
	infer(e->children[0], ret);

	// reset
	env = oldEnv;
	stack.resize(nstack);
}

void Analysis::_infer (AST::MethodExp* e, TypePtr dest)
{
	TypePtr selfty = nullptr;

	if (dest->kind == Type::Concrete)
	{
		if (dest->base->isFunction() && dest->args.size() > 1)
			selfty = dest->args[0];
	}

	if (!selfty)
		selfty = Type::poly();

	auto typeName = selfty->str();

	Env::Function* fn;
	auto fnty = _findMethod(selfty, e->name, fn);

	// no such method
	// TODO: this will happen often if 'selfty' is completely unknown
	//       show a more helpful error message?
	if (fnty == nullptr)
	{
		std::ostringstream ss;
		ss << "undefined method '" << e->name << "'";
		throw SourceError(ss.str(),
			{ "type: " + typeName }, e->span);
	}

	// reconstruct 'fnty' with 'selfty' prepended to arguments
	auto argc = fnty->args.size() - 1;
	auto fnbase = Env::Type::function(argc + 1);
	auto fnty2 = Type::concrete(fnbase, TypeList(selfty, fnty->args));

	// finish things
	unify(dest, fnty2, e->span);
	e->method = fn;
}

void Analysis::_infer (AST::MatchExp* e, TypePtr dest)
{
	auto condtype = Type::poly();
	infer(e->children[0], condtype);

	auto old = stackSave();

	for (size_t i = 1, len = e->children.size(); i < len; i++)
	{
		infer(e->patterns[i - 1], condtype);
		infer(e->children[i], dest);

		stackRestore(old);
	}
}

void Analysis::_infer (AST::ReturnExp* e)
{
	infer(e->children[0], ret);
}

void Analysis::_infer (AST::LetExp* e)
{
	// determine type using init + pattern matching
	TypePtr type = Type::poly();
	infer(e->children[0], type);

	// this also creates necessary variables
	infer(e->pattern, type);
}

void Analysis::_infer (AST::AssignExp* e)
{
	auto type = Type::poly();

	// unify left and right hand expressions
	auto lh = e->children[0];
	infer(lh, type);
	infer(e->children[1], type);

	if (auto lhvar = dynamic_cast<AST::VarExp*>(lh.get()))
	{
		// if left hand is a local variable, let it be
		//  known that it was mutated
		if (auto var = lhvar->var)
			var->didMut = true;
	}
}






void Analysis::_infer (AST::ConstPat* p, TypePtr dest)
{
	infer(p->exp, dest);

	if (dest->kind == Type::Concrete)
	{
		auto base = dest->base;
		p->equals = base->getMethod(Names::Equal);
	}
}
void Analysis::_infer (AST::BindPat* p, TypePtr dest)
{
	// just create a variable
	p->var = let(p->name, dest);
}
void Analysis::_infer (AST::EnumPat* p, TypePtr dest)
{
	// create temporary if needed
	if (!(p->args.empty() || p->rootPosition))
		p->var = temp();

	// do tuple things
	if (p->isTuple())
	{
		_inferTuplePat(p, dest);
		return;
	}

	// ensure it's actually a constructor
	auto glob = nm->getGlobal(p->name);
	if (glob == nullptr || !glob->isFunc ||
			glob->func->kind != Env::Function::EnumFunction)
	{
		std::ostringstream ss;
		ss << "invalid enum constructor '" << p->name.str() << "'";
		throw SourceError(ss.str(), p->span);
	}

	// ensure correctness
	auto ctor = glob->func;
	if (ctor->args.size() != p->args.size())
	{
		std::ostringstream ss1, ss2;
		ss1 << "expected: " << ctor->args.size();
		ss2 << "found: " << p->args.size();

		throw SourceError("invalid number of arguments to constructor",
			{ ss1.str(), ss2.str() }, p->span);
	}
	p->ctor = ctor;

	// get original enum type
	std::vector<TypePtr> with;
	auto ret = replaceParams(ctor->ret, with);
	unify(dest, ret, p->span);

	// infer arguments
	for (size_t i = 0, len = p->args.size(); i < len; i++)
	{
		auto argty = replaceParams(ctor->args[i].type, with);
		infer(p->args[i], argty);
	}
}
void Analysis::_inferTuplePat (AST::EnumPat* p, TypePtr dest)
{
	// extremely similar to infer(AST::TupleExp*, ...)
	TypeList args;
	size_t nargs = p->args.size();

	if (nargs == 0)
	{
		unify(dest, unitType, p->span);
		return;
	}

	for (size_t i = 0; i < nargs; i++)
		args = TypeList(Type::poly(), args);

	auto model = Type::concrete(Env::Type::tuple(nargs), args);
	unify(dest, model, p->span);

	for (size_t i = 0; i < nargs; ++args, i++)
		infer(p->args[i], args.head());
}




}}

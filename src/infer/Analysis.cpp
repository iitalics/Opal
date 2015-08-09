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

TypePtr Analysis::_inst (TypePtr obj, TypePtr type)
{
	std::vector<TypePtr> with;
	if (obj->kind != Type::Concrete ||
			obj->base->isIFace)
		with.push_back(obj);

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
			return _findIFaceFunc(obj, name, out);

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
		auto res = _findIFaceFunc(obj, name, out);
		if (res)
			return res;
	}
	return nullptr;
}
TypePtr Analysis::_findIFaceFunc (TypePtr obj, const std::string& name, Env::Function*& out)
{
	auto base = obj->base;

	for (size_t i = 0; i < base->iface.nfuncs; i++)
		if (base->iface.funcs[i].name == name)
		{
			out = nullptr; // TOOD: FIX THIS
			return _inst(obj, base->iface.funcs[i].getType());
		}

	return nullptr;
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

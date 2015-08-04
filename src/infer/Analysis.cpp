#include "Analysis.h"
#include "../env/Namespace.h"
namespace Opal { namespace Infer {
;


Analysis::Analysis (Env::Namespace* _nm,
		const std::vector<Var>& args)
	: nm(_nm), _ctx(_nm), _polyCount(0)
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
TypePtr Analysis::newType ()
{
	int id = _polyCount++;
	auto res = Type::poly(id, {});
	_polies.push_back(res);
	return res;
}

void Analysis::unify (TypePtr dest, TypePtr src, const Span& span)
{
	std::cout << "**  unify (" << dest->str() << ", " << src->str() << ")" << std::endl;

	if (src->kind == Type::Poly && dest->kind != Type::Poly)
	{
		auto t = dest;
		dest = src;
		src = t;
	}

	// TODO: ifaces
	//  needed: check if type has method
	//          instanciate parameter types into poly types
	//          alpha equivalence check

	if (dest->kind == Type::Poly)
	{
		// ifaces TODO: check if type subscribes
		//              to methods and unify things
		
		if (src->isPoly(dest->id))
			return; // unify(a, a)
		else if (src->containsPoly(dest->id))
			goto fail_self_ref;
		
		set(dest->id, src);
	}
	else if (dest->kind == Type::Concrete && src->kind == Type::Concrete)
	{
		// ifaces TODO: implicit cast?
		if (dest->base != src->base)
			goto fail_incompatible;
		
		for (auto xs = dest->args, ys = src->args; !xs.nil(); ++xs, ++ys)
			unify(xs.head(), ys.head(), span);
	}
	else if (dest->kind == Type::Param && src->kind == Type::Param)
	{
		if (dest->id != src->id)
			goto fail_incompatible;
	}
	else
		goto fail_incompatible;
	return;

fail_incompatible:
	throw SourceError("type inference error: incompatible types",
		{ "expected: " + dest->str(),
	      "found: " + src->str() }, span);

fail_self_ref:
	throw SourceError("type inference error: infinite type", span);
}
void Analysis::set (int polyId, TypePtr res)
{
	bool erase = (res->kind != Type::Poly);

	for (auto it = _polies.begin(); it != _polies.end(); ++it)
		if ((*it)->isPoly(polyId))
		{
			(*it)->set(res);
			if (erase)
				_polies.erase(it--);
		}
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
	else if (auto e2 = dynamic_cast<AST::CompareExp*>(e.get()))
		_infer(e2, dest);
	else if (auto e2 = dynamic_cast<AST::BlockExp*>(e.get()))
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
		// type = instParams(type)
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
}
void Analysis::_infer (AST::FieldExp* e, TypePtr dest)
{
}
void Analysis::_infer (AST::CompareExp* e, TypePtr dest)
{
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


}}

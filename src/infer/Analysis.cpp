#include "Analysis.h"
namespace Opal { namespace Infer {
;


Analysis::Analysis (Env::Namespace* _nm)
	: ctx(_nm), nm(_nm), _polyCount(0)
{}
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
	throw SourceError("type inference error: infinite type");
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

void Analysis::infer (AST::ExpPtr e, TypePtr ctx)
{
	// TODO: type inference HERE
}


}}

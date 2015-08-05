#include "Analysis.h"
#include "../env/Namespace.h"
namespace Opal { namespace Infer {
;

TypePtr Analysis::newType (const TypeList& ifaces)
{
	int id = _polyCount++;
	auto res = Type::poly(id, ifaces);
	_polies.push_back(res);
	return res;
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

TypePtr Analysis::replaceParams (TypePtr ty, std::vector<TypePtr>& with)
{
	auto newArgs = ty->args.map<TypePtr>([&] (TypePtr ty2) {
		return replaceParams(ty2, with);
	});

	if (ty->kind == Type::Param)
	{
		while (with.size() <= ty->id)
			with.push_back(nullptr);

		if (with[ty->id] == nullptr)
			with[ty->id] = newType(newArgs);
		
		return with[ty->id];
	}

	if (ty->kind == Type::Concrete)
	{
		if (ty->args.nil())
			return ty;
		else
			return Type::concrete(ty->base, newArgs);
	}
	else
		return ty;
}
void Analysis::polyToParam (TypePtr type)
{
	if (type->kind == Type::Poly)
	{
		std::ostringstream ss;
		ss << type->id;
		auto param = _ctx.createParam(ss.str(), type->args);
		
		set(type->id, param);
	}

	for (auto arg : type->args)
		polyToParam(arg);
}


void Analysis::unify (TypePtr dest, TypePtr src, const Span& span)
{
	switch (_unify(dest, src))
	{
	case FailBadMatch:
		throw SourceError("type matching failed: incompatible types",
			{ "expected: " + dest->str(),
		      "found: " + src->str() }, span);

	case FailInfinite:
		throw SourceError("type matching failed: infinite type", span);

	default: break;
	}
}
int Analysis::_unify (TypePtr dest, TypePtr src)
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
			return UnifyOK; // unify(a, a)
		else if (src->containsPoly(dest->id))
			return FailInfinite;
		
		set(dest->id, src);
		return UnifyOK;
	}
	else if (dest->kind == Type::Concrete && src->kind == Type::Concrete)
	{
		// ifaces TODO: implicit cast?
		if (dest->base != src->base)
			return FailBadMatch;
		
		int s;

		for (auto xs = dest->args, ys = src->args; !xs.nil(); ++xs, ++ys)
			if ((s = _unify(xs.head(), ys.head())) != UnifyOK)
				return s;

		return UnifyOK;
	}
	else if (dest->kind == Type::Param && src->kind == Type::Param)
	{
		if (dest->id != src->id)
			return FailBadMatch;
		else
			return UnifyOK;
	}
	else
		return FailBadMatch;
}



}}

#include "Analysis.h"
#include "../env/Namespace.h"
namespace Opal { namespace Infer {
;

TypePtr Analysis::replaceParams (TypePtr ty, std::vector<TypePtr>& with)
{
	auto newArgs = ty->args.map<TypePtr>([&] (TypePtr ty2) {
		return replaceParams(ty2, with);
	});

	if (ty->kind == Type::Param)
	{
		size_t idx = ty->id;

		while (with.size() <= idx)
			with.push_back(nullptr);

		if (with[idx] == nullptr)
			return (with[idx] = Type::poly(newArgs));
		else	
			return with[idx];
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

#define UNIFY_FAIL "type matching failed: "

static Span ifaceFuncSpan (TypePtr iface, const std::string& name)
{
	auto base = iface->base;
	for (size_t i = 0; i < base->iface.nfuncs; i++)
		if (base->iface.funcs[i].name == name)
			return base->iface.funcs[i].declSpan;
	return Span();
}

void Analysis::unify (TypePtr dest, TypePtr src, const Span& span)
{
	if (dest->kind == Type::Concrete && dest->base->isIFace)
	{
		// when you use an iface type as a normal concrete
		//  type, you can do implicit 'conversions'
		/*
			fn f (x : Show) { ... }

			let y = 4 : int
			f(y) // unify(Show, int) => ok;
		*/
		dest = Type::poly({ dest });
	}

	switch (_unify(dest, src))
	{
	case UnifyOK: break;

	case FailInfinite:
		throw SourceError(UNIFY_FAIL "infinite type", span);

	case FailSubscribe:
		{
			std::ostringstream ss1, ss2;

			ss1 << UNIFY_FAIL "type incompatible with iface '" << _failIFace->str() << "'";
			ss2 << "expected method: '" << _failName << "'";
			throw SourceError(ss1.str(),
				{ "type: " + _failType->str(), ss2.str() },
				{ span, ifaceFuncSpan(_failIFace, _failName) });
		}

	case FailMerge:
		{
			std::ostringstream ss;
			ss << UNIFY_FAIL "'" << _failIFace->str() << "' and '"
			   << _failIFace2->str() << "' are incompatible";

			throw SourceError(ss.str(),
					/*
				{ "conflicting method: " + _failName },
				{ ifaceFuncSpan(_failIFace, _failName),
				  ifaceFuncSpan(_failIFace2, _failName),
				  span }); */
				  span);
		}

	case FailBadMatch:
	default:
		throw SourceError(UNIFY_FAIL "incompatible types",
			{ "expected: " + dest->str(),
		      "found: " + src->str() }, span);
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

	if (dest->kind == Type::Poly && src->kind == Type::Poly)
	{
		// merge iface list when unifying two poly types
		// e.g. unify(_1, _2) => ok; _1 <- _2
		//      unify(_1(Ord), _2(Eq)) => ok;
		//          _1 <- _3(Ord, Show)
		//          _2 <- _3(Ord, Show)

		if (!_mergePoly(dest, src))
			return FailMerge;

		return UnifyOK;
	}
	else if (dest->kind == Type::Poly)
	{
		// set dest poly type to src
		// e.g. unify(_1, int) => ok; _1 <- int

		// type contains self: infinite type, e.g.
		//    let x = []   # x : list[_1]
		//    x = [x]      # _1 <- list[_1]
		if (src->containsPoly(dest))
			return FailInfinite;

		// check if src type subscribes to all ifaces in dest poly
		for (auto iface : dest->args)
			if (!_subscribes(iface, src))
				return FailSubscribe;
		
		// assign it
		dest->set(src);
		return UnifyOK;
	}
	else if (dest->kind == Type::Concrete && src->kind == Type::Concrete &&
				dest->base == src->base)
	{
		// merge two concrete types of same base
		// e.g. unify(list[int], list[_1]) => ok; _1 <- int

		int err;
		for (auto xs = dest->args, ys = src->args; !xs.nil(); ++xs, ++ys)
			if ((err = _unify(xs.head(), ys.head())) != UnifyOK)
				return err;

		return UnifyOK;
	}
	else if (dest->kind == Type::Param && src->kind == Type::Param)
	{
		// two params types of the same id
		// e.g. unify(#a, #b) => FAIL
		//      unify(#a, #a) => ok
		if (dest->id != src->id)
			return FailBadMatch;
		else
			return UnifyOK;
	}
	else
		// any other form is invalid
		// e.g. unify(#a, int) => FAIL
		//      unify(int, bool) => FAIL
		return FailBadMatch;
}
bool Analysis::_subscribes (TypePtr iface, TypePtr type)
{
	auto base = iface->base;

	// does 'type' subscribe to each method of 'iface'
	for (size_t i = 0; i < base->iface.nfuncs; i++)
	{
		Env::Function* _dummy;
		auto& method = base->iface.funcs[i];

		// find and inst the methods
		auto dest = _inst(iface, method.type, type);
		auto src = _findMethod(type, method.name, _dummy);

		// see if they are compatible
		if (src == nullptr || _unify(dest, src) != UnifyOK)
		{
			_failType = type;
			_failIFace = iface;
			_failName = method.name;
			return false;
		}
	}
	return true;
}
bool Analysis::_mergePolyAdd (TypeList& list, TypePtr iface)
{
	// TODO: make this better
	for (auto iface2 : list)
		if (iface2->base == iface->base)
		{
			for (auto xs = iface->args, ys = iface2->args; !xs.nil(); ++xs, ++ys)
				if (_unify(xs.head(), ys.head()) != UnifyOK)
				{
					_failIFace = iface2;
					_failIFace2 = iface;
					return false;
				}

			return true;
		}
	list = TypeList(iface, list);
	return true;
}
bool Analysis::_mergePoly (TypePtr a, TypePtr b)
{
	if (a->isPoly(b))
		return true;
	else if (a->args.nil())
	{
		a->set(b);
		return true;
	}
	else if (b->args.nil())
	{
		b->set(a);
		return true;
	}

	TypeList args;
	for (auto iface : a->args)
		if (!_mergePolyAdd(args, iface))
			return false;
	for (auto iface : b->args)
		if (!_mergePolyAdd(args, iface))
			return false;

	auto res = Type::poly(args);
	a->set(res);
	b->set(res);
	return true;
}


}}

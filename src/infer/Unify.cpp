#include "Analysis.h"
#include "../env/Namespace.h"
namespace Opal { namespace Infer {
;


#define UNIFY_FAIL "type matching failed: "

bool Analysis::debuggingEnabled = false;

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
			std::ostringstream ss1, ss2, ss3;

			Env::Function* fn;
			auto expectType = _findIFaceFunc(_failType, _failIFace, _failName, fn);

			ss1 << UNIFY_FAIL "incompatible with iface '" << _failIFace->str() << "'";
			ss2 << "type: " << _failType->str();
			ss3 << "no method '" << _failName << "' : " << expectType->str();
			throw SourceError(ss1.str(), { ss2.str(), ss3.str() },
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

	if (debuggingEnabled)
		std::cout << "" << dest->str() << " {<- unify} " << src->str() << std::endl;

	if (dest->kind == Type::Poly && src->kind == Type::Poly)
	{
		// merge iface list when unifying two poly types
		// e.g. unify(_1, _2) => ok; _1 <- _2
		//      unify(_1(Ord), _2(Eq)) => ok;
		//          _1 <- _3(Ord, Eq)
		//          _2 <- _3(Ord, Eq)

		if (!_merge(dest, src))
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

		// assign it
		auto ifaces = dest->args;
		dest->set(src);

		// check if src type subscribes to all ifaces in dest poly
		for (auto iface : ifaces)
			if (!_subscribes(iface, src))
				return FailSubscribe;
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
bool Analysis::_merge (TypePtr a, TypePtr b)
{
	// simple merges that don't require a lot of thought
	if (a->isPoly(b))
	{
		return true;
	}
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

	// complicated merge
	if (debuggingEnabled)
		std::cout << a->str() << " {merge} " << b->str() << std::endl;

	// accumulate each iface
	// TODO: sort by # of methods?
	//       ideally we want to eliminate anon ifaces ASAP

	auto argsA = a->args;
	auto argsB = b->args;

	Merge merge(this);

	for (auto iface : argsA)
		if (!merge.addIFace(a, iface))
			return false;
	for (auto iface : argsB)
		if (!merge.addIFace(b, iface))
			return false;

	auto result = merge.finish();
	if (debuggingEnabled)
		std::cout << "{merge =>} " << result->str() << std::endl;

	a->set(result);
	b->set(result);

	return true;
}
Analysis::Merge::Merge (Analysis* _parent)
	: parent(_parent) {}

bool Analysis::Merge::addIFace (TypePtr obj, TypePtr iface)
{
	bool anyNew = false;
	std::string name;
	TypePtr ty;

	if (debuggingEnabled)
		std::cout << "{merge +} " << iface->str() << std::endl;

	// merge ifaces by checking for method conflicts
	auto base = iface->base;
	for (size_t i = 0; i < base->iface.nfuncs; i++)
	{
		name = base->iface.funcs[i].name;
		ty = base->iface.funcs[i].type;
		ty = _inst(iface, ty, obj);

		auto find = methods.find(name);
		if (find == methods.end())
		{
			// no conflict, add to the list
			anyNew = true;
			methods[name] = ty;
		}
		else
		{
			// conflict????
			// TODO: check conflicts and resolve ugly conflict problems
			auto methodty = find->second;
			if (parent->_unify(ty, methodty) != UnifyOK)
				goto fail;
		}
	}

	// this check reduces SOME redundant ifaces
	if (anyNew)
		ifaces.push_back(iface);

	/* (maybe) TODO: solve the "merge problem"
		"merge three sets together by finding the smallest combination
		 of set unions that represent the super set."

		A : {a, b}
		B : {b, c}
		C : {c, d}
		super set : {a, b, c, d}

		merge(A, B, C) = (A + C)
		 ^               ^
		 TODO: make this algorithm
	*/
	return true;
fail:
	for (auto& iface1 : ifaces)
		if (iface1->base->getMethod(name) != nullptr)
		{
			parent->_failIFace = iface1;
			parent->_failIFace2 = iface;
			break;
		}

	return false;
}
TypePtr Analysis::Merge::finish ()
{ return Type::poly(TypeList(ifaces)); }


}}

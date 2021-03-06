#include "Analysis.h"
#include "../env/Env.h"
#include "../env/Namespace.h"
#include <algorithm>
namespace Opal { namespace Infer {
;

TypePtr Type::concrete (Env::Type* base, const TypeList& args)
{
	auto ty = std::make_shared<Type>(Concrete);
	ty->base = base;
	ty->args = args;
	return ty;
}
TypePtr Type::param (int id, const std::string& name,
		const TypeList& ifaces)
{
	auto ty = std::make_shared<Type>(Param);
	ty->id = id;
	ty->args = ifaces;
	ty->paramName = name;
	return ty;
}
TypePtr Type::poly (const TypeList& ifaces)
{
	auto ty = std::make_shared<Type>(Poly);
	ty->links = new TypeWeakList { ty.get() };
	ty->args = ifaces;

	static int k = 0;
	std::ostringstream ss;
	k++;
	if (Analysis::debuggingEnabled)
		ss << ++k;
	ty->paramName = ss.str();
	return ty;
}

Type::~Type ()
{
	if (kind == Poly && links != nullptr)
	{
		for (auto it = links->begin(); it != links->end(); ++it)
			if (*it == this)
			{
				links->erase(it);
				break;
			}

		if (links->empty())
			delete links;
	}
}



void Type::set (TypePtr other)
{
	if (kind == Poly)
	{
		if (Analysis::debuggingEnabled)
			std::cout << str() << " {<- set} " << other->str() << std::endl;

		auto _links = links;
		for (auto ty : *links)
			ty->_set(other);
		delete _links;
	}
	else
		_set(other);
}
void Type::_set (TypePtr other)
{
	if (other->kind == Concrete)
	{
		base = other->base;
	}
	else if (other->kind == Param)
	{
		paramName = other->paramName;
		id = other->id;
	}
	else
	{
		paramName = other->paramName;
		links = other->links;
		if (kind == Poly)
			links->push_back(this);
	}

	kind = other->kind;
	args = other->args;
}




using ConstTypes = std::vector<const Type*>;

static std::string typeStr (ConstTypes& history, const Type* type);

static std::string functionTypeStr (ConstTypes& history, const Type* type)
{
	std::ostringstream ss;
	auto xs = type->args;
	bool first = true;
	ss << "fn(";
	for (; !xs.nil(); ++xs)
	{
		auto ty = xs.head();

		if (xs.tail().nil())
		{
			ss << ") -> " << typeStr(history, ty.get());
			break;
		}

		if (first)
			first = false;
		else
			ss << ", ";
		ss << typeStr(history, ty.get());
	}
	return ss.str();
}
static std::string tupleTypeStr (ConstTypes& history, const Type* type)
{
	std::ostringstream ss;
	bool first = true;
	ss << "(";
	for (auto ty : type->args)
	{
		if (first)
			first = false;
		else
			ss << ", ";
		ss << typeStr(history, ty.get());
	}
	ss << ")";
	return ss.str();
}
static std::string concreteTypeStr (ConstTypes& history, const Type* type)
{
	std::ostringstream ss;
	ss << type->base->fullname().str();

	if (type->args.nil())
		return ss.str();

	bool first = true;
	ss << "[";
	for (auto ty : type->args)
	{
		if (first)
			first = false;
		else
			ss << ", ";
		ss << typeStr(history, ty.get());
	}
	ss << "]";
	return ss.str();
}
static std::string anonIFaceStr (ConstTypes& history, const Type* type)
{
	std::ostringstream ss;

	auto full = type->base->fullname().name;
	auto cl = full.front(), cr = full.back();

	auto arg = type->args.head();

	ss << cl << type->base->name << " "
	   << typeStr(history, arg.get()) << cr;
	return ss.str();
}
static std::string paramTypeStr (ConstTypes& history, const Type* type, const std::string& prefix)
{
	for (auto t : history)
		if (t == type)
			return prefix;

	history.push_back(type);

	if (type->args.nil())
		return prefix;
	else
		return prefix + tupleTypeStr(history, type);
}
static std::string typeStr (ConstTypes& history, const Type* type)
{
	if (type->kind == Type::Concrete)
	{
		if (type->base->isFunction())
			return functionTypeStr(history, type);
		else if (type->base->isTuple())
			return tupleTypeStr(history, type);
		else if (type->base->isAnonIFace())
			return anonIFaceStr(history, type);
		else
			return concreteTypeStr(history, type);
	}
	else
	{
		if (type->kind == Type::Param)
			return paramTypeStr(history, type, "#" + type->paramName);
		else
			return paramTypeStr(history, type, "_" + type->paramName);
	}
}
std::string Type::str() const
{
	ConstTypes history;
	return typeStr(history, this);
}






bool Type::containsParam (const std::string& name)
{
	if (kind == Param && paramName == name)
		return true;
	for (auto ty : args)
		if (ty->containsParam(name))
			return true;
	return false;
}
bool Type::containsParam (int _id)
{
	if (kind == Param && id == _id)
		return true;
	for (auto ty : args)
		if (ty->containsParam(_id))
			return true;
	return false;
}
bool Type::containsPoly (TypePtr poly)
{
	if (kind == Poly) // IM NOT SURE ABOUT THIS
		// the "better" solution would be to have a list of
		//  types we've already looked at but that's really
		//  space inefficient and this is very fast.
		// I don't want this "faster" solution to also be
		//  >incorrect< so I'm not sure if this code is ok.
		return isPoly(poly);
	for (auto ty : args)
		if (ty->containsPoly(poly))
			return true;
	return false;
}

bool Type::isPoly (TypePtr other) const
{
	return kind == Poly && links == other->links;
}
bool Type::isConcrete (Env::Type* _base) const
{
	return kind == Concrete && base == _base;
}


static SourceError UndefinedType (const AST::Name& name, const Span& span)
{
	std::ostringstream ss;
	ss << "undefined type '" << name.str() << "'";
	throw SourceError(ss.str(), span);
}

TypePtr Type::concreteFromAST (AST::ConcreteType* ct, Ctx& ctx)
{
	Env::Type* base;
	TypeList args;

	if (ct->is<AST::FuncType>())
		base = Env::Type::function(ct->subtypes.size() - 1);
	else if (ct->is<AST::TupleType>())
		base = Env::Type::tuple(ct->subtypes.size());
	else
		base = ctx.nm->getType(ct->name);

	if (base == nullptr)
		throw UndefinedType(ct->name, ct->span);

	if (ct->subtypes.size() != base->nparams)
	{
		std::ostringstream ss;
		ss << "expected " << base->nparams << " argument"
		   << (base->nparams == 1 ? "" : "s");
		throw SourceError("wrong number of arguments to type",
			ct->span);
	}

	for (auto it = ct->subtypes.rbegin(); it != ct->subtypes.rend(); ++it)
	{
		auto ty2 = fromAST(*it, ctx);
		args = TypeList(ty2, args);
	}

	return concrete(base, args);
}
TypePtr Type::paramFromAST (AST::ParamType* pt, Ctx& ctx)
{
	TypeList ifaces;

	for (size_t i = 0, len = ctx.params.size(); i < len; i++)
		if (ctx.params[i]->paramName == pt->name)
		{
			if (!pt->ifaces.empty())
				throw SourceError("parameter-type defined multiple times",
						{ ctx.spans[i], pt->span });
			else
				return ctx.params[i];
		}

	for (auto it = pt->ifaces.rbegin(); it != pt->ifaces.rend(); ++it)
	{
		auto ty2 = fromAST(*it, ctx);

		if (ty2->kind != Concrete ||
				!ty2->base->isIFace)
			throw SourceError("invalid iface type", (*it)->span);

		if (ty2->containsParam(pt->name))
			throw SourceError("parameter-type references self in ifaces",
				pt->span);

		for (auto ty3 : ifaces)
			if (ty3->isConcrete(ty2->base))
			{
				std::ostringstream ss;
				ss << "parameter-type iface '" << ty2->base->fullname().str()
				   << "' used more than once";
				throw SourceError(ss.str(), pt->span);
			}

		ifaces = TypeList(ty2, ifaces);
	}

	if (!ctx.allowNewTypes)
	{
		std::ostringstream ss;
		ss << "undefined parameter-type '#" << pt->name << "'";
		throw SourceError(ss.str(), pt->span);
	}

	return ctx.createParam(pt->name, ifaces, pt->span);
}



TypePtr Type::fromAST (AST::TypePtr ty, Type::Ctx& ctx)
{
	if (ty == nullptr)
		return poly();
	else if (auto ct = dynamic_cast<AST::ConcreteType*>(ty.get()))
		return concreteFromAST(ct, ctx);
	else if (auto pt = dynamic_cast<AST::ParamType*>(ty.get()))
		return paramFromAST(pt, ctx);
	else
		throw SourceError("unimplemented type", ty->span);
}
TypePtr Type::Ctx::createParam (const std::string& name, const TypeList& ifaces, const Span& span)
{
	int id = params.size();
	auto res = Type::param(id, name, ifaces);
	params.push_back(res);
	spans.push_back(span);
	return res;
}
void Type::Ctx::locateParams (TypePtr type)
{
	if (type->kind == Type::Param)
	{
		size_t idx = type->id;

		while (params.size() <= idx)
			params.push_back(nullptr);

		params[idx] = type;
	}
	else
		for (auto ty : type->args)
			locateParams(ty);
}



}}

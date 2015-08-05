#include "Type.h"
#include "../env/Env.h"
#include "../env/Namespace.h"
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
	ty->paramName = name;
	ty->args = ifaces;
	return ty;
}
TypePtr Type::poly (int id, const TypeList& ifaces)
{
	auto ty = std::make_shared<Type>(Poly);
	ty->id = id;
	ty->args = ifaces;
	return ty;
}

Type::~Type () {}

std::string Type::str() const
{
	std::ostringstream ss;

	char left, right;

	if (kind == Concrete)
	{
		ss << base->fullname().str();

		left = '[';
		right = ']';
	}
	else
	{
		if (kind == Param)
			ss << "#" << paramName;
		else
			ss << "_";// << id;

		left = '(';
		right = ')';
	}

	if (!args.nil())
	{
		ss << left;
		bool first = true;
		for (auto x : args)
		{
			if (first)
				first = false;
			else
				ss << ", ";
			ss << x->str();
		}
		ss << right;
	}

	return ss.str();
}
void Type::set (TypePtr other)
{
	kind = other->kind;
	args = other->args;
	if (other->kind == Concrete)
	{
		base = other->base;
	}
	else
	{
		if (other->kind == Param)
			paramName = other->paramName;

		id = other->id;
	}
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
bool Type::containsPoly (int _id)
{
	if (isPoly(_id))
		return true;
	for (auto ty : args)
		if (ty->containsPoly(_id))
			return true;
	return false;
}

bool Type::isPoly (int _id) const
{
	return kind == Poly && id == _id;
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

	std::cout << "creating param #" << pt->name << " -> "
	          << ctx.params.size() << std::endl;

	return ctx.createParam(pt->name, ifaces, pt->span);
}



TypePtr Type::fromAST (AST::TypePtr ty, Type::Ctx& ctx)
{
	if (auto ct = dynamic_cast<AST::ConcreteType*>(ty.get()))
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
		while (params.size() <= type->id)
			params.push_back(nullptr);

		params[type->id] = type;
	}
	else
		for (auto ty : type->args)
			locateParams(ty);
}



}}

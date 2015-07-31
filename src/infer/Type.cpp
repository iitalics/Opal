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

	if (kind == Concrete)
	{
		ss << base->fullname().str();
		if (!args.nil())
			ss << "["
			   << args.map<std::string>([] (TypePtr t) {
			   		return t->str();
			   }) << "]";
	}
	else
	{
		if (kind == Param)
			ss << "#" << paramName;
		else
			ss << "_";

		if (!args.nil())
			ss << "("
			   << args.map<std::string>([] (TypePtr t) {
			   		return t->str();
			   }) << ")";
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



static SourceError UndefinedType (const AST::Name& name, const Span& span)
{
	std::ostringstream ss;
	ss << "undefined type '" << name.str() << "'";
	throw SourceError(ss.str(), span);
}

TypePtr Type::fromAST (AST::TypePtr ty, Type::Ctx& ctx)
{
	if (auto ct = dynamic_cast<AST::ConcreteType*>(ty.get()))
	{
		auto base = ctx.nm->getType(ct->name);
		if (base == nullptr)
			throw UndefinedType(ct->name, ty->span);

		if (ct->subtypes.size() != base->nparams)
		{
			std::ostringstream ss;
			ss << "expected " << base->nparams << " argument"
			   << (base->nparams == 1 ? "" : "s");
			throw SourceError("wrong number of arguments to type",
				{ ss.str() },
				ty->span);
		}

		TypeList args;
		for (auto it = ct->subtypes.rbegin(); it != ct->subtypes.rend(); ++it)
		{
			auto ty2 = fromAST(*it, ctx);
			args = TypeList(ty2, args);
		}

		return concrete(base, args);
	}

	auto pt = (AST::ParamType*) (ty.get());

	for (size_t i = 0, len = ctx.params.size(); i < len; i++)
		if (ctx.params[i]->paramName == pt->name)
		{
			if (!pt->ifaces.empty())
				throw SourceError("parameter-type defined multiple times",
						{ ctx.spans[i], ty->span });
			else
				return ctx.params[i];
		}

	TypeList ifaces;
	for (auto it = pt->ifaces.rbegin(); it != pt->ifaces.rend(); ++it)
	{
		auto ty2 = fromAST(*it, ctx);
		ifaces = TypeList(ty2, ifaces);

		if (ty2->kind != Concrete ||
				!ty2->base->isIFace)
			throw SourceError("invalid iface type", (*it)->span);

		if (ty2->containsParam(pt->name))
			throw SourceError("parameter-type references self in ifaces",
				{ ty->span });
	}

	if (!ctx.allowNewTypes)
	{
		std::ostringstream ss;
		ss << "undefined parameter-type '#" << pt->name << "'";
		throw SourceError(ss.str(), ty->span);
	}

	int id = ctx.params.size();
	auto res = param(id, pt->name, ifaces);
	ctx.params.push_back(res);
	ctx.spans.push_back(ty->span);
	return res;
}




}}

#include "Type.h"
#include "../env/Env.h"
#include "../env/Namespace.h"
namespace Opal { namespace Infer {
;

TypePtr Type::concrete (Env::Type* base, const list<TypePtr>& args)
{
	auto ty = std::make_shared<Type>(Concrete);
	ty->base = base;
	ty->args = args;
	return ty;
}
TypePtr Type::param (int id, const std::string& name,
		const list<Env::Type*>& ifaces)
{
	auto ty = std::make_shared<Type>(Concrete);
	ty->id = id;
	ty->paramName = name;
	ty->ifaces = ifaces;
	return ty;
}
TypePtr Type::poly (int id, const list<Env::Type*>& ifaces)
{
	auto ty = std::make_shared<Type>(Poly);
	ty->id = id;
	ty->ifaces = ifaces;
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

		if (!ifaces.nil())
			ss << "("
			   << ifaces.map<std::string>([] (Env::Type* t) {
			   		return t->fullname().str();
			   }) << ")";
	}

	return ss.str();
}
void Type::set (TypePtr other)
{
	kind = other->kind;
	if (other->kind == Concrete)
	{
		base = other->base;
		args = other->args;
	}
	else
	{
		if (other->kind == Param)
			paramName = other->paramName;
		
		id = other->id;
		ifaces = other->ifaces;
		args = list<TypePtr>();
	}
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

		if (ct->subtypes.size() != base->args())
			throw SourceError("invalid number of arguments to type", ty->span);

		list<TypePtr> args;
		for (auto it = ct->subtypes.rbegin(); it != ct->subtypes.rend(); ++it)
		{
			auto ty2 = fromAST(*it, ctx);
			args = list<TypePtr>(ty2, args);
		}

		return concrete(base, args);
	}

	auto pt = (AST::ParamType*) (ty.get());

	for (size_t i = 0, len = ctx.params.size(); i < len; i++)
		if (ctx.params[i]->paramName == pt->name)
		{
			if (!pt->ifaces.empty())
				throw SourceError("parameter type defined multiple times",
						{ ctx.spans[i], ty->span });
			else
				return ctx.params[i];
		}

	list<Env::Type*> ifaces;
	for (auto it = pt->ifaces.rend(); it != pt->ifaces.rbegin(); ++it)
	{
		auto iface = ctx.nm->getType(*it);
		if (iface == nullptr)
			throw UndefinedType(*it, ty->span);

		ifaces = list<Env::Type*>(iface, ifaces);
	}

	int id = ctx.params.size();
	auto res = param(id, pt->name, ifaces);
	ctx.params.push_back(res);
	return res;
}




}}

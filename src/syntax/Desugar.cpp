#include "Desugar.h"
#include "../env/Loader.h"
namespace Opal { namespace Desugar {
;

static void desugarName (const AST::Name& name)
{
	if (name.hasModule())
		Env::loadModule(name.module);
}

void desugar (AST::TypePtr& ty)
{
	auto param = dynamic_cast<AST::ParamType*>(ty.get());
	if (param != nullptr)
	{
		for (auto& name : param->ifaces)
			desugarName(name);
		return;
	}

	auto conc = dynamic_cast<AST::ConcreteType*>(ty.get());
	if (conc != nullptr)
	{
		desugarName(conc->name);
		for (auto& ty2 : conc->subtypes)
			desugar(ty2);
	}
}
void desugar (AST::ExpPtr& e)
{
	for (auto& c : e->children)
		desugar(c);

	if (auto var = dynamic_cast<AST::VarExp*>(e.get()))
	{
		desugarName(var->name);
	}
	else if (auto let = dynamic_cast<AST::LetExp*>(e.get()))
	{
		if (let->varType != nullptr)
			desugar(let->varType);
	}
}



void desugar (AST::DeclPtr& decl)
{
	if (auto fndecl = dynamic_cast<AST::FuncDecl*>(decl.get()))
	{
		if (fndecl->impl.type != nullptr)
			desugar(fndecl->impl.type);
		for (auto& a : fndecl->args)
			desugar(a.type);
		desugar(fndecl->body);
	}
	else if (auto tydecl = dynamic_cast<AST::TypeDecl*>(decl.get()))
	{
		if (tydecl->alias != nullptr)
			desugar(tydecl->alias);
		for (auto& m : tydecl->members)
			desugar(m.type);
	}
	else if (auto constdecl = dynamic_cast<AST::ConstDecl*>(decl.get()))
	{
		if (constdecl->type != nullptr)
			desugar(constdecl->type);
		else
			desugar(constdecl->init);
	}
	else if (auto ifacedecl = dynamic_cast<AST::IFaceDecl*>(decl.get()))
	{
		for (auto& fn : ifacedecl->funcs)
		{
			for (auto& a : fn.args)
				desugar(a.type);
			desugar(fn.ret);
		}
	}
}

}}

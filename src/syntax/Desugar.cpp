#include "Desugar.h"
#include "../env/Loader.h"
namespace Opal { namespace Desugar {
;


#define SUG_MEMBER_SET       "set"
#define SUG_MEMBER_GET       "get"
#define SUG_COMPARE          "cmp"
#define SUG_EQUAL            "equal"



static void desugarName (const AST::Name& name)
{
	if (name.hasModule())
		Env::loadModule(name.module);
}

void desugar (AST::TypePtr& ty)
{
	if (auto param = dynamic_cast<AST::ParamType*>(ty.get()))
	{
		for (auto& ty2 : param->ifaces)
			desugar(ty2);
		return;
	}
	else if (auto conc = dynamic_cast<AST::ConcreteType*>(ty.get()))
	{
		desugarName(conc->name);
		for (auto& ty2 : conc->subtypes)
			desugar(ty2);
	}
}
void desugar (AST::ExpPtr& e)
{
	if (auto var = dynamic_cast<AST::VarExp*>(e.get()))
	{
		desugarName(var->name);
	}
	else if (auto assign = dynamic_cast<AST::AssignExp*>(e.get()))
	{
		auto lh = assign->children[0];
		auto rh = assign->children[1];

		if (auto mem = dynamic_cast<AST::MemberExp*>(lh.get()))
		{
			auto obj = mem->children[0];
			auto what = mem->children[1];
			auto span = assign->span;

			// a[b] = c   ->   a.set(b, c)
			e = AST::methodCall(span, obj, SUG_MEMBER_SET, { what, rh });
		}
		else if (!(lh->is<AST::VarExp>() || lh->is<AST::FieldExp>()))
			throw SourceError("invalid left-hand of assignment", lh->span);
	}
	else if (auto mem = dynamic_cast<AST::MemberExp*>(e.get()))
	{
		auto obj = mem->children[0];
		auto what = mem->children[1];
		auto span = mem->span;

		// a[b]   ->   a.get(b)
		e = AST::methodCall(span, obj, SUG_MEMBER_GET, { what });
	}
	else if (auto cmp = dynamic_cast<AST::CompareExp*>(e.get()))
	{
		AST::ExpPtr res;
		auto a = cmp->children[0];
		auto b = cmp->children[1];
		auto span = cmp->span;

		// a == b   ->   a.equal(b) == true
		// a != b   ->   a.equal(b) == false
		// a > b    ->   a.cmp(b) > 0
		// a < b    ->   a.cmp(b) < 0
		// etc.
		if (cmp->kind == AST::CompareExp::Eq ||
				cmp->kind == AST::CompareExp::NotEq)
			res = AST::methodCall(span, a, SUG_EQUAL, { b });
		else
			res = AST::methodCall(span, a, SUG_COMPARE, { b });

		e = AST::ExpPtr(new AST::CompareExp(res, cmp->kind));
		e->span = span;
	}
	else if (auto obj = dynamic_cast<AST::ObjectExp*>(e.get()))
	{
		desugar(obj->objType);
	}

	for (auto& c : e->children)
		desugar(c);
}



void desugar (AST::DeclPtr& decl)
{
	if (auto fndecl = dynamic_cast<AST::FuncDecl*>(decl.get()))
	{
		if (fndecl->impl.type != nullptr)
			desugar(fndecl->impl.type);
		for (auto& a : fndecl->args)
			desugar(a.type);

		if (fndecl->isExtern)
			desugar(fndecl->ret);
		else
			desugar(fndecl->body);
	}
	else if (auto tydecl = dynamic_cast<AST::TypeDecl*>(decl.get()))
	{
		for (auto& m : tydecl->fields)
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
			for (auto& ty : fn.args)
				desugar(ty);
			desugar(fn.ret);
		}
	}
}

}}

#include "Desugar.h"
#include "../env/Loader.h"
#include "../Names.h"
namespace Opal { namespace Desugar {
;


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
	auto span = e->span;

	if (auto var = dynamic_cast<AST::VarExp*>(e.get()))
	{
		desugarName(var->name);
	}
	else if (auto let = dynamic_cast<AST::LetExp*>(e.get()))
	{
		desugar(let->pattern);
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
			e = AST::methodCall(span, obj, Names::Set, { what, rh });
		}
		else if (!(lh->is<AST::VarExp>() || lh->is<AST::FieldExp>()))
			throw SourceError("invalid left-hand of assignment", lh->span);
	}
	else if (auto mem = dynamic_cast<AST::MemberExp*>(e.get()))
	{
		auto obj = mem->children[0];
		auto what = mem->children[1];

		// a[b]   ->   a.get(b)
		e = AST::methodCall(span, obj, Names::Get, { what });
	}
	else if (auto cmp = dynamic_cast<AST::CompareExp*>(e.get()))
	{
		AST::ExpPtr res;
		auto a = cmp->children[0];
		auto b = cmp->children[1];

		// a == b   ->   a.equal(b) == true
		// a != b   ->   a.equal(b) == false
		// a > b    ->   a.cmp(b) > 0
		// a < b    ->   a.cmp(b) < 0
		// etc.
		if (cmp->kind == AST::CompareExp::Eq ||
				cmp->kind == AST::CompareExp::NotEq)
			res = AST::methodCall(span, a, Names::Equal, { b });
		else
			res = AST::methodCall(span, a, Names::Compare, { b });

		e = AST::ExpPtr(new AST::CompareExp(res, cmp->kind));
		e->span = span;
	}
	else if (auto obj = dynamic_cast<AST::ObjectExp*>(e.get()))
	{
		desugar(obj->objType);
	}
	else if (auto lam = dynamic_cast<AST::LambdaExp*>(e.get()))
	{
		for (auto arg : lam->args)
			if (arg.type != nullptr)
				desugar(arg.type);
	}
	else if (dynamic_cast<AST::ConsExp*>(e.get()))
	{
		auto Cons = AST::ExpPtr(new AST::VarExp(AST::Name(Names::Cons, "Core")));
		e = AST::ExpPtr(new AST::CallExp(Cons, e->children));
		e->span = Cons->span = span;
	}
	else if (dynamic_cast<AST::NilExp*>(e.get()))
	{
		auto Nil = AST::ExpPtr(new AST::VarExp(AST::Name(Names::Nil, "Core")));
		e = AST::ExpPtr(new AST::CallExp(Nil, {}));
		e->span = Nil->span = span;
	}
	else if (dynamic_cast<AST::ListExp*>(e.get()))
	{
		auto elems = e->children;

		auto res = AST::ExpPtr(new AST::NilExp());
		res->span = span;
		for (size_t i = 0, len = elems.size(); i < len; i++)
		{
			auto elem = elems[len - i - 1];
			res = AST::ExpPtr(new AST::ConsExp(elem, res));
			res->span = elem->span;
		}

		e = res;
		desugar(e); // again!
	}

	for (auto& c : e->children)
		desugar(c);
}
void desugar (AST::PatPtr& p)
{
	if (auto epat = dynamic_cast<AST::EnumPat*>(p.get()))
	{
		desugarName(epat->name);
		for (auto& p2 : epat->args)
			desugar(p2);
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

		if (fndecl->isExtern)
			desugar(fndecl->ret);
		else
			desugar(fndecl->body);
	}
	else if (auto tydecl = dynamic_cast<AST::TypeDecl*>(decl.get()))
	{
		for (auto& m : tydecl->fields)
			desugar(m.type);
		for (auto& efn : tydecl->enumfns)
			for (auto& ty : efn.args)
				desugar(ty);
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

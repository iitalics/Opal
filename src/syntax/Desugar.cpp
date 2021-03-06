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
		bool okay = false;

		if (auto mem = dynamic_cast<AST::MemberExp*>(lh.get()))
		{
			auto obj = mem->children[0];
			auto what = mem->children[1];
			auto span = assign->span;

			if (mem->kind == AST::MemberExp::Get)
			{
				// a[b] = c   ->   a.set(b, c)
				e = AST::methodCall(span, obj, Names::Set, { what, rh });
				okay = true;
			}
		}
		else if (lh->is<AST::VarExp>() || lh->is<AST::PropertyExp>())
			okay = true;

		if (!okay)
			throw SourceError("invalid left-hand of assignment", lh->span);
	}
	else if (auto mem = dynamic_cast<AST::MemberExp*>(e.get()))
	{
		auto obj = mem->children[0];
		auto args = mem->children;

		// a[b]   ->   a.get(b)
		// a[b,]  ->   a.sub_from(b)
		// a[,b]  ->   a.sub_to(b)
		// a[b,c] ->   a.sub(b, c)
		switch (mem->kind)
		{
		case AST::MemberExp::Slice:
			e = AST::methodCall(span, obj, Names::Slice, { args[1], args[2] });
			break;
		case AST::MemberExp::SliceFrom:
			e = AST::methodCall(span, obj, Names::SliceFrom, { args[1] });
			break;
		case AST::MemberExp::SliceTo:
			e = AST::methodCall(span, obj, Names::SliceTo, { args[1] });
			break;
		case AST::MemberExp::Get:
		default:
			e = AST::methodCall(span, obj, Names::Get, { args[1] });
			break;
		}
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
	else if (dynamic_cast<AST::ListExp*>(e.get()))
	{
		auto elems = e->children;

		// build linked list right to left
		auto res = AST::Exp::nil(span);

		for (size_t i = 0, len = elems.size(); i < len; i++)
		{
			auto elem = elems[len - i - 1];
			res = AST::Exp::cons(elem->span, elem, res);
		}

		e = res;
		desugar(e); // again!
		return;
	}
	else if (auto block = dynamic_cast<AST::BlockExp*>(e.get()))
	{
		for (auto& c : e->children)
		{
			if (c == block->last)
			{
				desugar(block->last);
				c = block->last;
			}
			else
				desugar(c);
		}
		return;
	}
	else if (auto match = dynamic_cast<AST::MatchExp*>(e.get()))
	{
		for (auto& pat : match->patterns)
			desugar(pat);
	}

	for (auto& c : e->children)
		desugar(c);
}
void desugar (AST::PatPtr& p)
{
	if (auto epat = dynamic_cast<AST::EnumPat*>(p.get()))
	{
		if (epat->kind == AST::EnumPat::Enum)
			desugarName(epat->name);

		for (auto& p2 : epat->args)
			desugar(p2);

		// build linked list right to left
		if (epat->kind == AST::EnumPat::List)
		{
			auto res = AST::Pat::nil(epat->span);

			for (size_t i = 0, len = epat->args.size(); i < len; i++)
			{
				auto elem = epat->args[len - i - 1];
				res = AST::Pat::cons(elem->span, elem, res);
			}
			res->rootPosition = epat->rootPosition;

			p = res;
		}
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

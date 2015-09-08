#include "CodeGen.h"
namespace Opal { namespace Code {
;

using Cmd = Run::Cmd;

static inline int loadCmd (Infer::LocalVar* var)
{ return var->needsBox() ? Cmd::BoxLoad : Cmd::Load; }
static inline int storeCmd (Infer::LocalVar* var)
{ return var->needsBox() ? Cmd::BoxStore : Cmd::Store; }


CodeGen::CodeGen (Env::Function* func)
	: _mod(func->module), _nargs(func->args.size()), _localEnv(func->localEnv)
{
	for (size_t i = 0; i < _nargs; i++)
	{
		auto arg = _localEnv->defs[i];
		if (arg->needsBox())
			add({ Cmd::Box, .var = var(arg) });
	}

	_patFail = -1;

	generate(func->body);
	add(Cmd::Ret);

	if (_patFail != -1)
	{
		place(Label(_patFail));
		add({ Cmd::String, .string = new std::string("MatchFail") });
		add(Cmd::Throw);
	}

	std::cout << "generated code: " << func->fullname().str() << std::endl;
	showCode();
}
CodeGen::~CodeGen () {}

Run::Code CodeGen::generate (Env::Function* func)
{
	CodeGen codegen(func);
	return codegen.output();
}

CodeGen::Label CodeGen::label ()
{
	_labels.push_back(-1);
	return _labels.size() - 1;
}
void CodeGen::place (Label label)
{
	if (label < _labels.size())
		_labels[label] = _program.size();
}
CodeGen::Label CodeGen::patFail ()
{
	if (_patFail == -1)
		_patFail = label();

	return Label(_patFail);
}
size_t CodeGen::var (Infer::LocalVar* var)
{
	return _localEnv->index(var);
}
Run::Code CodeGen::output ()
{
	auto prgm = new Cmd[_program.size()];

	for (size_t i = 0, len = _program.size(); i < len; i++)
	{
		prgm[i] = _program[i];

		// label linkage
		if (_program[i].cmd == Cmd::Jump ||
				_program[i].cmd == Cmd::Else)
			prgm[i].dest_pc = _labels[_program[i].index];
	}

	return Run::Code(prgm,
		_localEnv->refs.size() + _nargs,
		_localEnv->defs.size() - _nargs);
}
bool CodeGen::_anyOutput (AST::ExpPtr e)
{
	if (e->is<AST::LetExp>() ||
			e->is<AST::AssignExp>() ||
			e->is<AST::WhileExp>())
		return false;
	if (e->is<AST::CondExp>() && e->children.size() < 3)
		return false;
	return true;
}

static inline SourceError unimplement (const Span& span)
{
	return SourceError("cannot generate this expression", span);
}

void CodeGen::generate (AST::ExpPtr e)
{
	if (auto e2 = dynamic_cast<AST::BlockExp*>(e.get())) _generate(e2);
	else if (auto e2 = dynamic_cast<AST::VarExp*>(e.get())) _generate(e2);
	else if (auto e2 = dynamic_cast<AST::NumberExp*>(e.get())) _generate(e2);
	else if (auto e2 = dynamic_cast<AST::CallExp*>(e.get())) _generate(e2);
	else if (auto e2 = dynamic_cast<AST::LetExp*>(e.get())) _generate(e2);
	else if (auto e2 = dynamic_cast<AST::AssignExp*>(e.get())) _generate(e2);
	else if (auto e2 = dynamic_cast<AST::CondExp*>(e.get())) _generate(e2);
	else if (auto e2 = dynamic_cast<AST::LazyOpExp*>(e.get())) _generate(e2);
	else if (auto e2 = dynamic_cast<AST::CompareExp*>(e.get())) _generate(e2);
	else if (auto e2 = dynamic_cast<AST::FieldExp*>(e.get())) _generate(e2);
	else if (auto e2 = dynamic_cast<AST::ReturnExp*>(e.get())) _generate(e2);
	else if (auto e2 = dynamic_cast<AST::TupleExp*>(e.get())) _generate(e2);
	else if (auto e2 = dynamic_cast<AST::ObjectExp*>(e.get())) _generate(e2);
	else if (auto e2 = dynamic_cast<AST::LambdaExp*>(e.get())) _generate(e2);
	else if (auto e2 = dynamic_cast<AST::MethodExp*>(e.get())) _generate(e2);
	else if (auto e2 = dynamic_cast<AST::MatchExp*>(e.get())) _generate(e2);

	else if (auto e2 = dynamic_cast<AST::TypeHintExp*>(e.get()))
		generate(e2->children[0]);
	else if (auto e2 = dynamic_cast<AST::StringExp*>(e.get()))
		add({ Cmd::String, .string = new std::string(e2->value) });
	else if (auto e2 = dynamic_cast<AST::BoolExp*>(e.get()))
		add(e2->value ? Cmd::True : Cmd::False);
	else if (auto e2 = dynamic_cast<AST::CharExp*>(e.get()))
		add({ Cmd::Char, .char_val = e2->value });

	else
		throw unimplement(e->span);
}
void CodeGen::generate (AST::PatPtr p, size_t _else)
{
	if (auto p2 = dynamic_cast<AST::ConstPat*>(p.get())) _generate(p2, _else);
	else if (auto p2 = dynamic_cast<AST::BindPat*>(p.get())) _generate(p2, _else);
	else if (auto p2 = dynamic_cast<AST::EnumPat*>(p.get())) _generate(p2, _else);
	else
		throw unimplement(p->span);
}

void CodeGen::_generate (AST::BlockExp* e)
{
	bool unit = true;

	for (auto e2 : e->children)
	{
		generate(e2);

		if (e2 == e->last)
			unit = false;
		else if (_anyOutput(e2))
			add(Cmd::Drop);
	}

	if (unit)
		add(Cmd::Unit);
}
void CodeGen::_generate (AST::VarExp* e)
{
	if (e->global != nullptr)
	{
		if (e->global->isFunc)
		{
			add({ Cmd::Int, .int_val = 0 });
			add({ Cmd::Func, .func = e->global->func });
		}
		else
			add({ Cmd::GetGlob, .global = e->global });
	}
	else
		add({ loadCmd(e->var), .var = var(e->var) });
}
void CodeGen::_generate (AST::NumberExp* e)
{
	switch (e->kind)
	{
	case AST::NumberExp::Int:
		add({ Cmd::Int, .int_val = e->intValue });
		break;

	case AST::NumberExp::Real:
		add({ Cmd::Real, .real_val = e->realValue });
		break;

	case AST::NumberExp::Long:
		add({ Cmd::Long, .long_val = e->longValue });
		break;

	default:
		add(Cmd::Unit);
		break;
	}
}
void CodeGen::_generate (AST::CallExp* e)
{
	auto fne = e->children[0];
	Env::Function* func = e->function;

	if (func == nullptr)
	{
		generate(fne);
		add(Cmd::Prelude);
	}
	else if (fne->is<AST::FieldExp>())
	{
		generate(fne->children[0]);
	}

	for (size_t i = 1, len = e->children.size(); i < len; i++)
		generate(e->children[i]);

	if (func == nullptr)
		add({ Cmd::Apply, .count = e->children.size() - 1 });
	else
		add({ Cmd::Call, .func = func });
}
void CodeGen::_generate (AST::LetExp* e)
{
	// generate initialization
	generate(e->children[0]);

	// match against pattern
	generate(e->pattern,
		e->pattern->canFail() ? patFail() : 0);
}
void CodeGen::_generate (AST::AssignExp* e)
{
	auto lh = e->children[0];

	if (auto field = dynamic_cast<AST::FieldExp*>(lh.get()))
	{
		if (field->method != nullptr)
			throw SourceError("cannot assign to method", e->span);

		generate(field->children[0]);
	}

	generate(e->children[1]);

	if (auto fieldexp = dynamic_cast<AST::FieldExp*>(lh.get()))
	{
		add({ Cmd::Set, .index = size_t(fieldexp->index) });
	}
	else if (auto varexp = dynamic_cast<AST::VarExp*>(lh.get()))
	{
		if (varexp->global)
		{
			if (varexp->global->isFunc)
				throw SourceError("cannot assign to global function", e->span);

			add({ Cmd::SetGlob, .global = varexp->global });
		}
		else
			add({ storeCmd(varexp->var), .var = var(varexp->var) });
	}
	else
		add(Cmd::Drop);
}
void CodeGen::_generate (AST::CondExp* e)
{
	auto cond = e->children[0];
	auto labelElse = label();
	auto labelEnd = label();
	bool noValue = (e->children.size() == 2);

	generate(cond);
	add({ Cmd::Else, .index = labelElse });
	generate(e->children[1]);

	if (noValue)
		add(Cmd::Drop);
	else
		add({ Cmd::Jump, .index = labelEnd });

	place(labelElse);

	if (!noValue)
		generate(e->children[2]);

	place(labelEnd);
}
void CodeGen::_generate (AST::LazyOpExp* e)
{
	auto a = e->children[0];
	auto b = e->children[1];
	auto labelEnd = label();
	auto labelThen = label();

	generate(a);
	add(Cmd::Dupl);
	if (e->kind == AST::LazyOpExp::And)
	{
		add({ Cmd::Else, .index = labelEnd });
		//add({ Cmd::Jump, .index = labelThen });
	}
	else
	{
		add({ Cmd::Else, .index = labelThen });
		add({ Cmd::Jump, .index = labelEnd });
	}
	place(labelThen);
	add(Cmd::Drop);
	generate(b);
	place(labelEnd);
}
void CodeGen::_generate (AST::CompareExp* e)
{
	generate(e->children[0]);

	int flags = 0;
	switch (e->kind)
	{
	case AST::CompareExp::Lt:
		flags = Cmd::CmpLt; break;
	case AST::CompareExp::LtEq:
		flags = Cmd::CmpLt | Cmd::CmpEq; break;
	case AST::CompareExp::Gr:
		flags = Cmd::CmpGr; break;
	case AST::CompareExp::GrEq:
		flags = Cmd::CmpGr | Cmd::CmpEq; break;
	case AST::CompareExp::NotEq:
		flags = Cmd::CmpNot; break;

	case AST::CompareExp::Eq:
	default:
		return;
	}
	add({ Cmd::Compare, .cmp_flags = flags });
}
void CodeGen::_generate (AST::FieldExp* e)
{
	if (e->method != nullptr)
	{
		generate(e->children[0]);
		add({ Cmd::Int, .int_val = 1 });
		add({ Cmd::Func, .func = e->method });
	}
	else
	{
		generate(e->children[0]);
		add({ Cmd::Get, .index = size_t(e->index) });
	}
}
void CodeGen::_generate (AST::ReturnExp* e)
{
	generate(e->children[0]);
	add(Cmd::Ret);
}
void CodeGen::_generate (AST::TupleExp* e)
{
	if (e->children.empty())
	{
		add(Cmd::Unit);
		return;
	}

	for (auto& e2 : e->children)
		generate(e2);
	add({ Cmd::Tuple, .count = e->children.size() });
}
void CodeGen::_generate (AST::ObjectExp* e)
{
	auto base = e->base;
	size_t nfields = base->data.nfields;

	add({ Cmd::Object, .type = base });
	for (size_t i = 0; i < nfields; i++)
	{
		add(Cmd::Dupl);
		generate(e->children[i]);
		add({ Cmd::Set, .index = size_t(e->index[i]) });
	}
}
void CodeGen::_generate (AST::LambdaExp* e)
{
	auto lamEnv = e->env;
	e->env = nullptr;

	// create lambda
	auto fn = _mod->makeLambda(e->span);
	for (size_t i = 0, len = e->args.size(); i < len; i++)
		fn->args.push_back({ e->args[i].name, lamEnv->defs[i]->type });

	fn->body = e->children[0];
	fn->localEnv = lamEnv;

	// generate code for it
	fn->code = generate(fn);

	// generate code to make the lambda
	for (auto ref : lamEnv->refs)
		add({ Cmd::Load, .var = var(ref) });
	add({ Cmd::Int, .int_val = Int_t(lamEnv->refs.size()) });
	add({ Cmd::Func, .func = fn });
}
void CodeGen::_generate (AST::MethodExp* e)
{
	add({ Cmd::Int, .int_val = 0 });
	add({ Cmd::Func, .func = e->method });
}
void CodeGen::_generate (AST::MatchExp* e)
{
	size_t ncases = e->patterns.size();

	generate(e->children[0]);
	Label next, end;

	end = label();

	for (size_t i = 0; i < ncases; i++)
	{
		if (i > 0)
			place(next);

		if (i < (ncases - 1))
			next = label();
		else if (e->patterns[i]->canFail())
			next = patFail();
		else
			next = 0;

		generate(e->patterns[i], next);
		generate(e->children[i + 1]);
		if (i < (ncases - 1))
			add({ Cmd::Jump, .index = end });
	}
	place(end);
}




void CodeGen::_generate (AST::ConstPat* p, size_t _else)
{
	if (p->equals == nullptr)
		throw SourceError("cannot compare to this constant", p->span);


	if (p->rootPosition)
		add(Cmd::Dupl);
	generate(p->exp);

	add({ Cmd::Call, .func = p->equals });
	add({ Cmd::Else, .index = _else });

	if (p->rootPosition)
		add(Cmd::Drop);
}
void CodeGen::_generate (AST::BindPat* p, size_t _else)
{
	add({ Cmd::Store, .var = var(p->var) });
	if (p->var->needsBox())
		add({ Cmd::Box, .var = var(p->var) });
}
void CodeGen::_generate (AST::EnumPat* p, size_t _else)
{
	if (p->var != nullptr)
		add({ Cmd::Store, .var = var(p->var) });

	if (p->ctor != nullptr)
	{
		if (p->var != nullptr)
			add({ Cmd::Load, .var = var(p->var) });
		else if (p->rootPosition)
			add(Cmd::Dupl);
		
		add({ Cmd::IsEnum, .func = p->ctor });
		add({ Cmd::Else, .index = _else });
	}

	for (size_t i = 0, len = p->args.size(); i < len; i++)
	{
		if (p->var != nullptr)
			add({ Cmd::Load, .var = var(p->var) });
		else if (p->rootPosition)
			add(Cmd::Dupl);

		add({ Cmd::Get, .index = i });
		generate(p->args[i], _else);
	}

	if (p->rootPosition)
		add(Cmd::Drop);
}



}}

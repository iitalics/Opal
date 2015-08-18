#include "CodeGen.h"
namespace Opal { namespace Code {
;

using Cmd = Run::Cmd;

CodeGen::CodeGen (Env::Function* func)
	: _nargs(func->args.size()), _nvars(0)
{
	generate(func->body);
	add(Cmd::Ret);

	std::cout << "generated code: " << func->fullname().str() << std::endl;
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
size_t CodeGen::var (int varID)
{
	size_t var = varID;
	if (var >= _nargs)
	{
		if (_nvars < var - _nargs + 1)
			_nvars = var - _nvars + 1;
	}
	return var;
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

	return Run::Code(prgm, _nargs, _nvars);
}
bool CodeGen::_noValue (AST::ExpPtr e)
{
	if (e->is<AST::LetExp>() ||
			e->is<AST::AssignExp>() ||
			e->is<AST::WhileExp>())
		return true;
	if (e->is<AST::CondExp>() && e->children.size() < 3)
		return true;
	return false;
}

static inline SourceError cannot (const Span& span)
{
	return SourceError("cannot generate this expression", span);
}

void CodeGen::generate (AST::ExpPtr e)
{
	if (auto e2 = dynamic_cast<AST::BlockExp*>(e.get()))
		_generate(e2);
	else if (auto e2 = dynamic_cast<AST::VarExp*>(e.get()))
		_generate(e2);
	else if (auto e2 = dynamic_cast<AST::IntExp*>(e.get()))
		_generate(e2);
	else if (auto e2 = dynamic_cast<AST::RealExp*>(e.get()))
		_generate(e2);
	else if (auto e2 = dynamic_cast<AST::BoolExp*>(e.get()))
		_generate(e2);
	else if (auto e2 = dynamic_cast<AST::CallExp*>(e.get()))
		_generate(e2);
	else if (auto e2 = dynamic_cast<AST::LetExp*>(e.get()))
		_generate(e2);
	else if (auto e2 = dynamic_cast<AST::AssignExp*>(e.get()))
		_generate(e2);
	else if (auto e2 = dynamic_cast<AST::CondExp*>(e.get()))
		_generate(e2);
	else
		throw cannot(e->span);
}
void CodeGen::_generate (AST::BlockExp* e)
{
	bool drop = false;

	for (auto e2 : e->children)
	{
		if (drop)
			add(Cmd::Drop);
		generate(e2);
		drop = !_noValue(e2);
	}
	if (e->unitResult)
		add(Cmd::Unit);
}
void CodeGen::_generate (AST::VarExp* e)
{
	if (e->global != nullptr)
		add({ Cmd::SetGlob, .global = e->global });
	else
		add({ Cmd::Load, .var = var(e->varId) });
}
void CodeGen::_generate (AST::IntExp* e)
{
	if (e->castReal)
		add({ Cmd::Real, .real_val = Real_t(e->value) });
	else
		add({ Cmd::Int, .int_val = e->value });
}
void CodeGen::_generate (AST::RealExp* e)
{
	add({ Cmd::Real, .real_val = e->value });
}
void CodeGen::_generate (AST::BoolExp* e)
{
	add(e->value ? Cmd::True : Cmd::False);
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
	if (e->varType != nullptr) // default initializer
		throw cannot(e->span);

	generate(e->children[0]);
	add({ Cmd::Store, .var = var(e->varId) });
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
				throw SourceError("cannot assign to global function");

			add({ Cmd::SetGlob, .global = varexp->global });
		}
		else
			add({ Cmd::Store, .var = var(varexp->varId) });
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




}}

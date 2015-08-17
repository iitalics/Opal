#include "CodeGen.h"
namespace Opal { namespace Code {
;



CodeGen::CodeGen (Env::Function* func)
	: nargs(func->args.size()), nvars(0)
{
	generate(func->body);
	add(Run::Cmd::Ret);

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
	labels.push_back(-1);
	return labels.size() - 1;
}
void CodeGen::place (Label label)
{
	if (label < labels.size())
		labels[label] = program.size();
}
Run::Code CodeGen::output ()
{
	auto prgm = new Run::Cmd[program.size()];

	for (size_t i = 0, len = program.size(); i < len; i++)
	{
		prgm[i] = program[i];

		// label linkage
		if (program[i].cmd == Run::Cmd::Jump ||
				program[i].cmd == Run::Cmd::Else)
			prgm[i].dest_pc = labels[program[i].index];
	}

	return Run::Code(prgm, nargs, nvars);
}

void CodeGen::generate (AST::ExpPtr e)
{
	// ...
}




}}

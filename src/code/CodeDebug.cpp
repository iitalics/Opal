#include "CodeGen.h"
namespace Opal { namespace Code {
;

void CodeGen::showCode ()
{
	std::vector<std::string> cmd_names {
		"noop", "load", "store", "dupl",
		"drop", "int", "real", "long",
		"string", "unit", "true", "false",
		"char", "jump", "else", "compare",
		"isenum", "call", "tail", "prelude",
		"apply", "get", "set", "getglob",
		"setglob", "object", "tuple", "func",
		"ret", "throw"
	};

	std::vector<std::string> var_names;
	for (auto ref : _localEnv->refs)
		var_names.push_back(ref->name);
	for (auto def : _localEnv->defs)
		var_names.push_back(def->name);

	std::cout << "{" << std::endl;

	size_t pc = 0;
	for (auto cmd : _program)
	{
		for (size_t i = 0; i < _labels.size(); i++)
			if (_labels[i] == pc)
				std::cout << "/" << i << std::endl;
		pc++;

		std::cout << "  " << cmd_names[cmd.cmd];
		switch (cmd.cmd)
		{
		case Run::Cmd::Load:
		case Run::Cmd::Store:
			std::cout << " %" << var_names[cmd.index] << std::endl;
			break;

		case Run::Cmd::Get:
		case Run::Cmd::Set:
		case Run::Cmd::Apply:
		case Run::Cmd::Tuple:
			std::cout << " " << cmd.index << std::endl;
			break;

		case Run::Cmd::Jump:
		case Run::Cmd::Else:
			std::cout << " /" << cmd.index << std::endl;
			break;

		case Run::Cmd::Int:
			std::cout << " (" << cmd.int_val << ")" << std::endl; break;
		case Run::Cmd::Real:
			std::cout << " (" << cmd.real_val << ")" << std::endl; break;
		case Run::Cmd::Long:
			std::cout << " (" << cmd.long_val << "L)" << std::endl; break;
		case Run::Cmd::String:
			std::cout << " \"" << *(cmd.string) << "\"" << std::endl; break;
		case Run::Cmd::Char:
			std::cout << " '" << cmd.char_val << "'" << std::endl; break;
		case Run::Cmd::Compare:
			std::cout << " (";
			if (cmd.cmp_flags & Run::Cmd::CmpNot) std::cout << "not";
			if (cmd.cmp_flags & Run::Cmd::CmpLt) std::cout << "<";
			if (cmd.cmp_flags & Run::Cmd::CmpGr) std::cout << ">";
			if (cmd.cmp_flags & Run::Cmd::CmpEq) std::cout << "=";
			std::cout << ")" << std::endl;
			break;

		case Run::Cmd::Object:
			std::cout << " <" << cmd.type->fullname().str() << ">" << std::endl;
			break;

		case Run::Cmd::GetGlob:
		case Run::Cmd::SetGlob:
			std::cout << " <" << cmd.global->fullname().str() << ">" << std::endl;
			break;

		case Run::Cmd::Call:
		case Run::Cmd::Tail:
		case Run::Cmd::IsEnum:
		case Run::Cmd::Func:
			std::cout << " <" << cmd.func->fullname().str() << ">" << std::endl;
			break;

		case Run::Cmd::NoOp:
		case Run::Cmd::Dupl:
		case Run::Cmd::Drop:
		case Run::Cmd::Unit:
		case Run::Cmd::True:
		case Run::Cmd::False:
		case Run::Cmd::Prelude:
		case Run::Cmd::Ret:
		case Run::Cmd::Throw:
		default:
			std::cout << std::endl;
		}
	}

	std::cout << "}" << std::endl;
}


}}

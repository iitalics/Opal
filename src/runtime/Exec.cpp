#include "Exec.h"
#include "../env/Env.h"
namespace Opal { namespace Run {
;


Code::Code (Cmd* _program, size_t _nargs, size_t _nvars)
	: program(_program), nargs(_nargs), nvars(_nvars) {}
Code::~Code () { }


Thread::Thread () {}
Thread::~Thread ()
{
	drop(_stack.size());
}


/// stack utilities
///
Cell Thread::get (size_t pos)
{
	if (_stack.size() > pos)
		return _stack[pos];
	else
		return Cell::Unit();
}
void Thread::set (size_t pos, Cell cell)
{
	if (_stack.size() > pos)
	{
		_stack[pos].release();
		_stack[pos] = cell.retain();
	}
}
Cell Thread::pop ()
{
	if (!_stack.empty())
	{
		auto cell = _stack.back();
		_stack.pop_back();
		return cell;
	}
	else
		return Cell::Unit();
}
void Thread::drop ()
{
	if (!_stack.empty())
	{
		_stack.back().release();
		_stack.pop_back();
	}
}
void Thread::push (Cell cell)
{
	_stack.push_back(cell.retain());
}
void Thread::units (size_t n)
{
	_stack.reserve(_stack.size() + n);
	for (size_t i = 0; i < n; i++)
		_stack.push_back(Cell::Unit());
}
void Thread::drop (size_t n)
{
	if (n > _stack.size())
		n = _stack.size();

	for (size_t i = 0; i < n; i++)
	{
		_stack.back().release();
		_stack.pop_back();
	}
}

void Thread::call (const Code& code)
{
	_calls.push_back(Exec {});
	_calls.back().begin(*this, code);
}
void Thread::call (Env::Function* func)
{
	switch (func->kind)
	{
	case Env::Function::CodeFunction:
		call(func->code);
		break;

	case Env::Function::NativeFunction:
		func->nativeFunc(*this);
		break;

	case Env::Function::IFaceFunction:
		throw SourceError("iface call unimplemented", func->declSpan);
	case Env::Function::EnumFunction:
		throw SourceError("enum call unimplemented", func->declSpan);
	default:
		throw SourceError("call unimplemented", func->declSpan);
	}
}
void Thread::ret ()
{
	_calls.pop_back();
}
bool Thread::step ()
{
	if (_calls.empty())
		return false;
	else
	{
		_calls.back().step(*this);
		return true;
	}
}

void Exec::begin (Thread& th, const Code& code)
{
	frame_pos = th.size() - code.nargs;
	th.units(code.nvars);
	program = code.program;
	pc = 0;
}
void Exec::step (Thread& th)
{
	std::string cmds[] = {
		"noop", "load", "store", "dupl",
		"drop", "int", "real", "unit",
		"true", "false", "jump", "else",
		"compare", "isenum", "call", "tail",
		"prelude", "apply", "get", "set",
		"getg", "setg", "make",	"ret",
		"throw"
	};

	auto cur = program[pc++];
	std::cout << "[" << (pc - 1) << "] " << cmds[cur.cmd];

	switch (cur.cmd)
	{
	case Cmd::Ret:
		std::cout << std::endl;
		th.ret();
		break;

	case Cmd::Get: case Cmd::Set:
	case Cmd::Apply:
		std::cout << " " << cur.index << std::endl;
		break;

	case Cmd::Load:
	case Cmd::Store:
		std::cout << " %" << cur.var << std::endl;
		break;

	case Cmd::Int:
		std::cout << " (" << cur.int_val << ")" << std::endl;
		break;
	case Cmd::Real:
		std::cout << " (" << cur.real_val << ")" << std::endl;
		break;

	case Cmd::Jump:
	case Cmd::Else:
		std::cout << " [" << cur.dest_pc << "]" << std::endl;
		break;

	case Cmd::Compare:
		std::cout << " {";
		if (cur.cmp_flags & Cmd::CmpNot)
			std::cout << "not";
		if (cur.cmp_flags & Cmd::CmpLt)
			std::cout << "<";
		if (cur.cmp_flags & Cmd::CmpGr)
			std::cout << ">";
		if (cur.cmp_flags & Cmd::CmpEq)
			std::cout << "=";
		std::cout << "}" << std::endl;
		break;

	case Cmd::Call:
	case Cmd::Tail:
	case Cmd::IsEnum:
		std::cout << " <" << cur.func->fullname().str() << ">" << std::endl;
		break;
	case Cmd::SetGlob: case Cmd::GetGlob:
		std::cout << " <" << cur.global->fullname().str() << ">" << std::endl;
		break;
	case Cmd::Make:
		std::cout << " <" << cur.type->fullname().str() << ">" << std::endl;
		break;

	default:
		std::cout << std::endl;
		break;
	}
}



}}

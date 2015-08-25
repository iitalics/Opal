#include "Exec.h"
#include "../env/Env.h"
namespace Opal { namespace Run {
;


Code::Code (Cmd* _program, size_t _nargs, size_t _nvars)
	: program(_program), nargs(_nargs), nvars(_nvars) {}
void Code::destroy ()
{
	delete[] program;
	program = nullptr;
}


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
//		std::cout << "{thread} popped: " << cell.str() << std::endl;
		return cell;
	}
	else
		return Cell::Unit();
}
void Thread::drop ()
{
	if (!_stack.empty())
	{
//		std::cout << "{thread} dropped: " << _stack.back().str() << std::endl;
		_stack.back().release();
		_stack.pop_back();
	}
}
void Thread::push (Cell cell)
{
//	std::cout << "{thread} pushed: " << cell.str() << std::endl;
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
//		std::cout << "{thread} dropped: " << _stack.back().str() << std::endl;
		_stack.back().release();
		_stack.pop_back();
	}
}
void Thread::remove (size_t st, size_t end)
{
	if (st >= _stack.size())
		st = _stack.size() - 1;
	if (end <= st)
		return;

//	std::cout << "{thread} remove " << st << " -> " << end << std::endl;

	for (size_t i = st; i < end; i++)
	{
//		std::cout << "{thread} dropped: " << _stack[i].str() << std::endl;
		_stack[i].release();
	}

	_stack.erase(_stack.begin() + st, _stack.begin() + end);
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
	static std::string cmds[] = {
		"noop", "load", "store", "dupl",
		"drop", "int", "real", "string", "unit",
		"true", "false", "jump", "else",
		"compare", "isenum", "call", "tail",
		"prelude", "apply", "get", "set",
		"getg", "setg", "object", "tuple",
		"func", "ret", "throw"
	};

	auto cur = program[pc++];
//	std::cout << "[" << (pc - 1) << "] " << cmds[cur.cmd] << std::endl;

	Cell a, b;

	switch (cur.cmd)
	{
	case Cmd::Int:
		th.push(a = Cell::Int(cur.int_val));
		break;
	case Cmd::Real:
		th.push(a = Cell::Real(cur.real_val));
		break;

	// UNIMPLEMENTED VALUE RETURNING
	case Cmd::GetGlob:
	case Cmd::Func:
	//

	case Cmd::Unit:
		th.push(a = Cell::Unit());
		break;
	case Cmd::True:
		th.push(a = Cell::Bool(true));
		break;
	case Cmd::False:
		th.push(a = Cell::Bool(false));
		break;
	case Cmd::String:
		th.push(a = Cell::String(*cur.string));
		a.release();
		break;
	case Cmd::Load:
		th.push(a = th.get(frame_pos + cur.index));
		break;
	case Cmd::Store:
		th.set(frame_pos + cur.index, a = th.pop());
		a.release();
		break;
	case Cmd::Dupl:
		th.push(a = th.get(th.size() - 1));
		break;
	case Cmd::Drop:
		th.drop();
		break;
	case Cmd::Jump:
		pc = cur.dest_pc;
		break;
	case Cmd::Else:
		a = th.pop();
		if (!a.dataBool)
			pc = cur.dest_pc;
		a.release();
		break;
	case Cmd::Compare:
		a = th.pop();
		th.push(b = Cell::Bool(
			((cur.cmp_flags & Cmd::CmpLt) && a.dataInt < 0) ||
			((cur.cmp_flags & Cmd::CmpGr) && a.dataInt > 0) ||
			((cur.cmp_flags & Cmd::CmpEq) && a.dataInt == 0) ||
			((cur.cmp_flags & Cmd::CmpNot) && !a.dataBool)));
		a.release();
		break;
	case Cmd::IsEnum:
		a = th.pop();
		th.push(b = Cell::Bool(a.isEnum(cur.func)));
		a.release();
		break;
	case Cmd::Call: // easy pz
		th.call(cur.func);
		break;
	case Cmd::Tail:
		{
			size_t argc = cur.func->args.size();
			th.remove(frame_pos, th.size() - argc);
			th.call(cur.func);
			break;
		}
	case Cmd::Prelude:
		a = th.pop();
		for (auto ch : a.simple->children)
			th.push(ch);
		th.push(a);
		a.release();
		break;
	case Cmd::Apply:
		{
			size_t argc = cur.count;
			size_t idx = th.size() - 1 - argc;
			a = th.get(idx);
			th.remove(idx, idx + 1);
			th.call(a.simple->ctor);
			a.release();
			break;
		}
	case Cmd::Get:
		a = th.pop();
		th.push(a.simple->get(cur.index));
		a.release();
		break;
	case Cmd::Set:
		b = th.pop();
		a = th.pop();
		a.simple->set(cur.index, b);
		a.release();
		b.release();
		break;
	case Cmd::SetGlob:
		break;
	case Cmd::Object:
		a = Cell::Enum(cur.type, cur.type->data.nfields);
		th.push(a);
		a.release();
		break;
	case Cmd::Tuple:
		{
			size_t n = cur.count;
			a = Cell::Enum(Env::Type::tuple(n), n);
			for (size_t i = n; i-- > 0; )
			{
				b = th.pop();
				a.simple->set(i, b);
				b.release();
			}
			th.push(a);
			a.release();
		}
	case Cmd::Ret:
		th.remove(frame_pos, th.size() - 1);
		th.ret();
		break;
	case Cmd::Throw:
		{
			std::ostringstream ss;
			a = th.pop();
			ss << "exception thrown: " << a.str();
			a.release();
			throw SourceError(ss.str());
		}
	case Cmd::NoOp:
	default:
		break;
	}
}



}}

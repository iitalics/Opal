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
Cell Thread::get (size_t pos) const
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
Cell Thread::peek () const
{
	if (!_stack.empty())
		return _stack.back();
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
Cell Thread::make (Env::Type* ty, size_t nfields, Env::Function* ctor)
{
	auto obj = Cell::Enum(ty, nfields, ctor);
	for (size_t i = 0; i < nfields; i++)
	{
		auto a = pop();
		obj.simple->set(nfields - 1 - i, a);
		a.release();
	}
	return obj;
}
void Thread::call (const Code& code)
{
	_calls.push_back(Exec());
	_calls.back().begin(*this, code);
}
void Thread::call (Env::Function* func)
{
	switch (func->kind)
	{
	case Env::Function::CodeFunction:
		call(func->code);
		_calls.back().caller = func;
		break;

	case Env::Function::NativeFunction:
		func->nativeFunc(*this);
		break;

	case Env::Function::IFaceFunction:
		{
			auto name = func->ifaceSig->name;
			auto argc = func->ifaceSig->argc;

			auto self = get(size() - argc - 1);
			auto actualType = self.type;

			for (auto m : actualType->methods)
				if (m->name == name)
				{
					call(m);
					return;
				}

			// well formed native code should prevent this from
			//  ever happening
			throw SourceError("cannot find iface function: " + name,
				{ "of type: " + actualType->fullname().str() },
				func->declSpan);
		}
	case Env::Function::EnumFunction:
		{
			// just make an object
			auto obj = make(func->enumType, func->args.size(), func);
			push(obj);
			obj.release();
		}
		break;

	default:
		throw SourceError("call unimplemented", func->declSpan);
	}
}
void Thread::ret ()
{
	_calls.pop_back();
}
void Thread::die (const std::string& name, const std::vector<Cell>& args)
{
	throw Error(name, args, *this);
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

Exec::Exec ()
	: program(nullptr), caller(nullptr) {}
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
		"noop", "load", "store", "dupl", "drop",
		"int", "real", "long", "string", "unit",
		"true", "false", "char", "jump", "else",
		"compare", "isenum", "call", "tail",
		"prelude", "apply", "get", "set",
		"getg", "setg", "object", "tuple",
		"func", "ret", "throw",
		"box", "boxload", "boxstore"
	};

	auto cur = program[pc++];
//	std::cout << "[" << (pc - 1) << "] " << cmds[cur.cmd] << std::endl;

	// general use variables
	Cell a, b;
	size_t n;

	switch (cur.cmd)
	{
	case Cmd::Int:
		th.push(a = Cell::Int(cur.int_val));
		break;
	case Cmd::Real:
		th.push(a = Cell::Real(cur.real_val));
		break;
	case Cmd::Long:
		th.push(a = Cell::Long(cur.long_val));
		break;
	case Cmd::Char:
		th.push(a = Cell::Char(cur.char_val));
		break;
	case Cmd::True:
		th.push(a = Cell::Bool(true));
		break;
	case Cmd::False:
		th.push(a = Cell::Bool(false));
		break;

	// UNIMPLEMENTED VALUE RETURNING
	case Cmd::GetGlob:
	//

	case Cmd::Unit:
		th.push(a = Cell::Unit());
		break;
	case Cmd::String:
		th.push(a = Cell::String(*cur.string));
		a.release();
		break;
	case Cmd::Load:
		th.push(a = th.get(frame_pos + cur.var));
		break;
	case Cmd::Store:
		th.set(frame_pos + cur.var, a = th.pop());
		a.release();
		break;
	case Cmd::Dupl:
		th.push(a = th.peek());
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
		n = cur.func->args.size(); // argc
		th.remove(frame_pos, th.size() - n);
		th.call(cur.func);
		break;
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
			a = th.get(idx).retain();
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
		n = cur.count;
		a = th.make(Env::Type::tuple(n), n);
		th.push(a);
		a.release();
		break;
	case Cmd::Func:
		b = th.pop();
		n = b.dataInt;
		b.release();
		a = th.make(Env::Type::function(cur.func->args.size()), n, cur.func);
		th.push(a);
		a.release();
		break;
	case Cmd::Box:
		a = th.get(frame_pos + cur.var);
		b = Cell::Box(a);
		th.set(frame_pos + cur.var, b);
		b.release();
		break;
	case Cmd::BoxLoad:
		b = th.get(frame_pos + cur.var);
		a = b.simple->get(0);
		th.push(a);
		break;
	case Cmd::BoxStore:
		a = th.pop();
		b = th.get(frame_pos + cur.var);
		b.simple->set(0, a);
		a.release();
		break;
	case Cmd::Ret:
		th.remove(frame_pos, th.size() - 1);
		th.ret();
		break;
	case Cmd::Throw:
		th.die("CodeError");
		break;

	case Cmd::NoOp:
	default:
		break;
	}
}



}}

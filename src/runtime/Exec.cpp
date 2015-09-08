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



std::vector<ThreadPtr> Thread::_threads;

ThreadPtr Thread::start ()
{
	ThreadPtr thread(new Thread());
	_threads.push_back(thread);
	return thread;
}
void Thread::stop (ThreadPtr thread)
{
	thread->_stopped = true;
}
const std::vector<ThreadPtr>& Thread::threads ()
{
	return _threads;
}
bool Thread::stepAny ()
{
	if (_threads.empty())
		return false;
	else
	{
		if (!_threads.back()->step())
			_threads.pop_back();

		return true;
	}
}

Thread::Thread ()
	: _stopped(false) {}
Thread::~Thread ()
{
	while (_stack.size() > 0)
		drop();
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
void Thread::remove (size_t st, size_t end)
{
	if (st >= _stack.size())
		st = _stack.size() - 1;
	if (end <= st)
		return;

	for (size_t i = st; i < end; i++)
		_stack[i].release();

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
	if (_stopped || _calls.empty())
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




}}

#include "Exec.h"
#include "../env/Env.h"
namespace Opal { namespace Run {
;

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
		th.push(b = Cell::Bool(a.ctor == cur.func));
		a.release();
		break;
	case Cmd::Call: // easy pz
		th.call(cur.func, cur.argc);
		break;
	case Cmd::Tail:
		th.remove(frame_pos, th.size() - cur.argc);
		th.call(cur.func, cur.argc);
		break;
	case Cmd::Prelude:
		a = th.pop();
		if (a.obj)
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
			th.call(a.ctor, argc);
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
		{
			a = th.pop();
			auto err = std::string(a.stringObj->string);
			a.release();
			th.die(err);
			break;
		}

	case Cmd::NoOp:
	default:
		break;
	}
}


}}

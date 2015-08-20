#pragma once
#include "../opal.h"
#include "Cell.h"
namespace Opal {
namespace Env {
class Type;
class Function;
class Global;
}
namespace Run {

class Thread;

// a single instruction
struct Cmd
{
	enum
	{
		// operations
		NoOp = 0,
		Load, Store, Dupl, Drop,
		Int, Real, String, Unit, True, False,
		Jump, Else, Compare, IsEnum,
		Call, Tail, Prelude, Apply,
		Get, Set, GetGlob, SetGlob,
		Make, Ret, Throw,

		CmpLt = 1,
		CmpGr = 2,
		CmpEq = 4,
		CmpNot = 8,
	};

	// operation
	int cmd;
	union
	{
		// operand
		Int_t int_val; // Int
		Real_t real_val; // Real
		int cmp_flags; // Compare

		size_t var; // Load, Store
		size_t dest_pc; // Jump, Else
		size_t count; // Apply
		size_t index; // Get, Set

		std::string* string;
		Env::Function* func; // Call, Tail, IsEnum
		Env::Type* type; // Make
		Env::Global* global; // GetGlob, SetGlob
	};
};

// callable function made from instructions
struct Code
{
	Code (Cmd* program, size_t nargs, size_t nvars);
	void destroy ();

	Cmd* program;
	size_t nargs;
	size_t nvars;
};

// executing frame
struct Exec
{
	size_t frame_pos;
	size_t pc;
	Cmd* program;

	void begin (Thread& th, const Code& code);
	void step (Thread& th);
};

// executing thread (with stacks)
class Thread
{
public:
	Thread ();
	~Thread ();

	bool step (); // returns false if nothing to do

	// stack utilities
	inline size_t size () const { return _stack.size(); }
	Cell get (size_t pos);
	void set (size_t pos, Cell cell);
	Cell pop ();
	void drop ();
	void drop (size_t n = 0);
	void push (Cell cell);
	void units (size_t n);

	// execution utilities
	void call (const Code& code);
	void call (Env::Function* func);
	void ret ();

private:
	std::vector<Exec> _calls;
	std::vector<Cell> _stack;
};



using NativeFn_t = void (*) (Thread&);

}}

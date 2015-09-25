#pragma once
#include "../opal.h"
#include "Cell.h"
#include <exception>
namespace Opal {
namespace Env {
class Type;
class Function;
class Global;
}
namespace Run {

class Thread;
using ThreadPtr = std::shared_ptr<Thread>;

// a single instruction
struct Cmd
{
	enum
	{
		// operations
		NoOp = 0,
		Load, Store, Dupl, Drop,
		Int, Real, Long, String, Unit,
		True, False, Char,
		Jump, Else, Compare, IsEnum,
		Call, Tail, Prelude, Apply,
		Get, Set, GetGlob, SetGlob,
		Object, Tuple, Func, Ret, Throw,
		Box, BoxLoad, BoxStore,

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
		Long_t long_val; // Long
		Char_t char_val; // Char
		int cmp_flags; // Compare

		size_t var; // Load, Store, Box, BoxLoad, BoxStore
		size_t dest_pc; // Jump, Else
		size_t count; // Apply, Tuple
		size_t index; // Get, Set

		std::string* string; // String
		Env::Function* func; // Call, Tail, IsEnum, Func
		Env::Type* type; // Object
		Env::Global* global; // GetGlob, SetGlob
	};
};

// callable function made from instructions
struct Code
{
	Code (Cmd* program, size_t nargs, size_t nvars);
	~Code ();

	Cmd* program;
	size_t nargs;
	size_t nvars;
};

// executing frame
struct Exec
{
	Exec ();

	size_t frame_pos;
	size_t pc;
	Cmd* program;
	Env::Function* caller;

	void begin (Thread& th, const Code& code);
	void step (Thread& th);
};

// executing thread (with stacks)
class Thread
{
public:
	static ThreadPtr start ();
	static void stop (ThreadPtr th);
	static const std::vector<ThreadPtr>& threads ();
	static bool stepAny ();

	~Thread ();

	bool step (); // returns false if nothing to do

	// stack utilities
	inline size_t size () const { return _stack.size(); }
	Cell get (size_t pos) const;
	void set (size_t pos, Cell cell);
	Cell pop ();
	Cell peek () const;
	void drop ();
	void push (Cell cell);
	void units (size_t n);
	void remove (size_t start, size_t end);
	Cell make (Env::Type* ty, size_t nfields,
				Env::Function* ctor = nullptr);

	// execution utilities
	void call (const Code& code);
	void call (Env::Function* func);
	void ret ();
	void die (const std::string& errName,
		const std::vector<Cell>& args = {});

private:
	static std::vector<ThreadPtr> _threads;
	Thread ();

	friend class Error;
	std::vector<Exec> _calls;
	std::vector<Cell> _stack;
	bool _stopped;
};

using NativeFn_t = void (*) (Thread&);

class Error : public std::exception
{
public:
	Error (const Error& other);
	virtual ~Error ();
	virtual const char* what () const noexcept;

	void operator= (const Error& e) = delete;
private:
	friend class Thread;
	Error (const std::string& name,
		const std::vector<Cell>& args,
		Thread& th);

	char* _what;
};


}}

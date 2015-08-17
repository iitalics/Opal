#pragma once
#include "../runtime/Exec.h"
#include "../env/Env.h"
#include "../syntax/AST.h"
namespace Opal { namespace Code {
;

class CodeGen
{
public:
	using Label = size_t;

	CodeGen (Env::Function* func);
	~CodeGen ();

	static Run::Code generate (Env::Function* func);

	inline void add (int cmd)
	{
		_program.push_back(Run::Cmd { cmd });
	}
	inline void add (const Run::Cmd& cmd)
	{
		_program.push_back(cmd);
	}
	Label label ();
	void place (Label label);
	size_t var (int varID);
	Run::Code output ();


	void generate (AST::ExpPtr e);
private:
	std::vector<Run::Cmd> _program;
	std::vector<size_t> _labels;
	size_t _nargs, _nvars;

	static bool _noValue (AST::ExpPtr e);
	void _generate (AST::BlockExp* e);
	void _generate (AST::VarExp* e);
	void _generate (AST::IntExp* e);
	void _generate (AST::RealExp* e);
	void _generate (AST::BoolExp* e);
	void _generate (AST::CallExp* e);
//	void _generate (AST::LetExp* e);
//	void _generate (AST::AssignExp* e);
//	void _generate (AST::LazyOpExp* e);
//	void _generate (AST::CompareExp* e);
//	void _generate (AST::FieldExp* e);
//	void _generate (AST::ReturnExp* e);
//	void _generate (AST::CondExp* e);
//	void _generate (AST::GotoExp* e);
//	void _generate (AST::StringExp* e);
//	void _generate (AST::TupleExp* e);
//	void _generate (AST::LambdaExp* e);
//	void _generate (AST::ObjectExp* e);
//	void _generate (AST::ConsExp* e);
//	void _generate (AST::NilExp* e);
//	void _generate (AST::WhileExp* e);
};

}}

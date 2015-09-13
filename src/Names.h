#pragma once
#include "opal.h"
namespace Opal { namespace Names {
;

// standardization of important
//  identifier names used throughout
//  the code

extern std::string
	// KEY      // VALUE
	Self,       // "self"

	Negate,     // "neg" (-)
	Not,        // "inv" (not)
	OperAdd,    // "add" (+)
	OperSub,    // "sub" (-)
	OperMul,    // "mul" (*)
	OperDiv,    // "div" (/)
	OperMod,    // "mod" (%)
	OperExp,    // "exp" (^)
	OperRBind,  // "rbind"  (|>)
	OperLBind,  // "lbind"  (<|)
	OperRBlock, // "rblock" (>|)
	OperLBlock, // "lblock" (|<)
	OperRShift, // "rshift" (>>)
	OperLShift, // "lshift" (<<)

	Compare,    // "cmp"
	Equal,      // "equal"
	Get,        // "get"
	Set,        // "set"

	Cons,       // "Cons"
	Nil;        // "Nil"



}}

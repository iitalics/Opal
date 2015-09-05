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

	Negate,     // "neg"
	Not,        // "not"
	OperAdd,    // "add"
	OperSub,    // "sub"
	OperMul,    // "mul"
	OperDiv,    // "div"
	OperMod,    // "mod"

	Compare,    // "cmp"
	Equal,      // "equal"
	Get,        // "get"
	Set,        // "set"

	Cons,       // "Cons"
	Nil;        // "Nil"



}}

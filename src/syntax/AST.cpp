#include "AST.h"
#include "../infer/Analysis.h"
namespace Opal { namespace AST {
;

bool Name::hasModule () const { return !module.empty(); }
std::string Name::str () const
{
	if (hasModule())
	{
		auto modName = module;
		if (module[0] == '.')
			modName = "(hidden)";
		return modName + "::" + name;
	}
	else
		return name;
}


Exp::~Exp () {}
VarExp::~VarExp () {}
NumberExp::~NumberExp () {}
void NumberExp::castReal ()
{
	kind = Real;
	realValue = Real_t(intValue);
}
void NumberExp::castLong ()
{
	kind = Long;
	longValue = Long_t(intValue);
}
void NumberExp::negate ()
{
	switch (kind)
	{
	case Int:  intValue  = -intValue; break;
	case Real: realValue = -realValue; break;
	case Long: longValue = -longValue; break;
	default: break;
	}
}
StringExp::~StringExp () {}
BoolExp::~BoolExp () {}
CharExp::~CharExp () {}
TupleExp::~TupleExp () {}
ListExp::~ListExp () {}
LambdaExp::~LambdaExp () { delete env; }
ObjectExp::ObjectExp (TypePtr _objType,
		const std::vector<Init>& _inits)
	: objType(_objType)
{
	children.reserve(_inits.size());
	inits.reserve(_inits.size());
	for (auto init : _inits)
	{
		inits.push_back(init.first);
		children.push_back(init.second);
	}
}
ObjectExp::~ObjectExp () {}
CondExp::~CondExp () {}
LetExp::~LetExp () {}
LazyOpExp::~LazyOpExp () {}
CompareExp::~CompareExp () {}
ConsExp::~ConsExp () {}
NilExp::~NilExp () {}
FieldExp::~FieldExp () {}
MethodExp::~MethodExp () {}
MemberExp::~MemberExp () {}
CallExp::~CallExp () {}
TypeHintExp::~TypeHintExp () {}
BlockExp::~BlockExp () {}
WhileExp::~WhileExp () {}
AssignExp::~AssignExp () {}
ReturnExp::~ReturnExp () {}
GotoExp::~GotoExp () {}

ExpPtr methodCall (const Span& span,
		ExpPtr obj, const std::string& method,
		const ExpList& args)
{
	ExpPtr mem(new FieldExp(obj, method));
	ExpPtr call(new CallExp(mem, args));
	mem->span = span;
	call->span = span;
	return call;
}
ExpPtr methodCall (ExpPtr obj, const std::string& method,
		const ExpList& args)
{
	return methodCall(obj->span, obj, method, args);
}



Pat::~Pat () {}
ConstPat::~ConstPat () {}
BindPat::~BindPat () {}
EnumPat::~EnumPat () {}
TuplePat::~TuplePat () {}




Type::Type () {}
Type::~Type () {}
std::string Type::str () const { return "<type>"; }
ParamType::~ParamType () {}
std::string ParamType::str () const
{
	std::ostringstream ss;
	ss << "#" << name;
	if (!ifaces.empty())
	{
		ss << "(";
		for (size_t i = 0, len = ifaces.size(); i < len; i++)
		{
			if (i > 0)
				ss << ", ";
			ss << ifaces[i]->str();
		}
		ss << ")";
	}
	return ss.str();
}
ConcreteType::~ConcreteType () {}
std::string ConcreteType::str () const
{
	std::ostringstream ss;
	ss << name.str();
	if (!subtypes.empty())
	{
		bool first = true;
		ss << "[";
		for (auto t : subtypes)
		{
			if (!first)
				ss << ", ";
			first = false;
			ss << t->str();
		}
		ss << "]";
	}
	return ss.str();
}
FuncType::~FuncType () {}
std::string FuncType::str () const
{
	std::ostringstream ss;
	ss << "fn(";
	for (size_t i = 0, len = subtypes.size(); i < len - 1; i++)
	{
		if (i > 0)
			ss << ", ";
		ss << subtypes[i]->str();
	}
	ss << ") -> " << subtypes.back()->str();
	return ss.str();
}
TupleType::~TupleType () {}
std::string TupleType::str () const
{
	std::ostringstream ss;
	bool first = true;
	ss << "(";
	for (auto t : subtypes)
	{
		if (!first)
			ss << ", ";
		first = false;
		ss << t->str();
	}
	ss << ")";
	return ss.str();
}

Decl::~Decl () {}
FuncDecl::~FuncDecl () {}
TypeDecl::~TypeDecl () {}
ConstDecl::~ConstDecl () {}
IFaceDecl::~IFaceDecl () {}
void IFaceDecl::add (const IFaceFunc& func)
{
	funcs.push_back(func);
}

Toplevel::Toplevel ()
	: module("") {}
Toplevel::~Toplevel () {}


}}

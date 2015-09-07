#include "AST.h"
#include "../infer/Analysis.h"
#include "../Names.h"
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
FieldExp::~FieldExp () {}
MethodExp::~MethodExp () {}
MemberExp::~MemberExp () {}
CallExp::~CallExp () {}
TypeHintExp::~TypeHintExp () {}
BlockExp::~BlockExp () {}
MatchExp::MatchExp (ExpPtr cond, const std::vector<Case>& cases)
	: Exp({ cond })
{
	children.reserve(cases.size() + 1);
	patterns.reserve(cases.size());
	for (auto pair : cases)
	{
		patterns.push_back(pair.first);
		children.push_back(pair.second);
	}
}
MatchExp::~MatchExp () {}
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

ExpPtr Exp::cons (Span sp, ExpPtr hd, ExpPtr tl)
{
	ExpPtr var(new VarExp(Name(Names::Cons, "Core")));
	ExpPtr res(new CallExp(var, { hd, tl }));
	var->span = res->span = sp;
	return res;
}
ExpPtr Exp::nil (Span sp)
{
	ExpPtr var(new VarExp(Name(Names::Nil, "Core")));
	ExpPtr res(new CallExp(var, { }));
	var->span = res->span = sp;
	return res;
}



Pat::~Pat () {}
bool Pat::canFail () const { return true; }
ConstPat::~ConstPat () {}
bool ConstPat::canFail () const { return true; }
BindPat::~BindPat () {}
EnumPat::~EnumPat () {}
bool EnumPat::canFail () const
{
	if (kind != Tuple)
		return true;

	for (auto arg : args)
		if (arg->canFail())
			return true;
	return false;
}

PatPtr Pat::cons (Span sp, PatPtr hd, PatPtr tl)
{
	PatPtr res(new EnumPat(Name(Names::Cons, "Core"), { hd, tl }));
	res->span = sp;
	return res;
}
PatPtr Pat::nil (Span sp)
{
	PatPtr res(new EnumPat(Name(Names::Nil, "Core"), {}));
	res->span = sp;
	return res;
}



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

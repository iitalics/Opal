#include <syntax/AST.h>

namespace AST
{

bool Name::hasModule () const { return !module.empty(); }
std::string Name::str () const
{
	if (hasModule())
		return module + "::" + name;
	else
		return name;
}

Exp::~Exp () {}
VarExp::~VarExp () {}
IntExp::~IntExp () {}
RealExp::~RealExp () {}
StringExp::~StringExp () {}
BoolExp::~BoolExp () {}
TupleExp::~TupleExp () {}
LambdaExp::~LambdaExp () {}
ObjectExp::~ObjectExp () {}
CondExp::~CondExp () {}
LetExp::~LetExp () {}
LazyOpExp::~LazyOpExp () {}
CompareExp::~CompareExp () {}
ConsExp::~ConsExp () {}
NilExp::~NilExp () {}
FieldExp::~FieldExp () {}
MemberExp::~MemberExp () {}
CallExp::~CallExp () {}
BlockExp::~BlockExp () {}
WhileExp::~WhileExp () {}
AssignExp::~AssignExp () {}
ReturnExp::~ReturnExp () {}
GotoExp::~GotoExp () {}

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
			ss << ifaces[i].str();
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
		ss << "[";
		bool first = true;
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


}
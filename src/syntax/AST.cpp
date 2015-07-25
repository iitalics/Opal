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

template <typename T>
static std::string to_string (T what)
{
	std::ostringstream ss;
	ss << what;
	return ss.str();
}
static inline std::string spaces (int k)
{
	return std::string(k, ' ');
}


Exp::~Exp () {}
std::string Exp::str (int ind) const { return "<exp>"; }
VarExp::~VarExp () {}
std::string VarExp::str (int ind) const { return name.str(); }
IntExp::~IntExp () {}
std::string IntExp::str (int ind) const { return to_string(value); }
RealExp::~RealExp () {}
std::string RealExp::str (int ind) const { return to_string(value); }
StringExp::~StringExp () {}
std::string StringExp::str (int ind) const { return "\"" + value + "\""; }
BoolExp::~BoolExp () {}
std::string BoolExp::str (int ind) const { return value ? "true" : "false"; }
TupleExp::~TupleExp () {}
std::string TupleExp::str (int ind) const
{
	std::ostringstream ss;
	ss << "(";
	for (size_t i = 0, len = children.size(); i < len; i++)
	{
		if (i > 0)
			ss << ", ";
		ss << children[i]->str(ind);
	}
	ss << ")";
	return ss.str();
}
ListExp::~ListExp () {}
std::string ListExp::str (int ind) const
{
	std::ostringstream ss;
	ss << "[";
	for (size_t i = 0, len = children.size(); i < len; i++)
	{
		if (i > 0)
			ss << ", ";
		ss << children[i]->str(ind);
	}
	ss << "]";
	return ss.str();
}
LambdaExp::~LambdaExp () {}
std::string LambdaExp::str (int ind) const
{
	std::ostringstream ss;
	ss << "fn (";
	for (size_t i = 0, len = args.size(); i < len; i++)
	{
		if (i > 0)
			ss << ", ";
		ss << args[i];
	}
	ss << ") ";
	if (!children[0]->is<BlockExp>())
		ss << "= ";
	ss << children[0]->str(ind);
	return ss.str();
}
ObjectExp::ObjectExp (TypePtr _objType,
		const std::vector<Init>& _inits)
	: objType(_objType)
{
	children.reserve(_inits.size());
	for (auto& init : _inits)
	{
		inits.push_back(init.first);
		children.push_back(init.second);
	}
}
ObjectExp::~ObjectExp () {}
std::string ObjectExp::str (int ind) const
{
	std::ostringstream ss;
	ss << "new " << objType->str() << " {" << std::endl;
	for (size_t i = 0, len = inits.size(); i < len; i++)
	{
		ss << spaces(ind + 2) << inits[i] << " = "
		   << children[i]->str(ind + 2) << std::endl;
	}
	ss << spaces(ind) << "}";
	return ss.str();
}
CondExp::~CondExp () {}
std::string CondExp::str (int ind) const
{
	std::ostringstream ss;
	ss << "if " << children[0]->str(ind + 2) << " "
	   << children[1]->str(ind);
	if (children.size() > 2)
	{
		ss << " else " << children[2]->str(ind);
	}
	return ss.str();
}
LetExp::~LetExp () {}
std::string LetExp::str (int ind) const
{
	std::ostringstream ss;
	ss << "let " << name;
	if (varType == nullptr)
		ss << " = " << children[0]->str(ind);
	else
		ss << " : " << varType->str();
	return ss.str();
}
LazyOpExp::~LazyOpExp () {}
std::string LazyOpExp::str (int ind) const
{
	std::ostringstream ss;
	ss << "(" << children[0]->str(ind) << ")";
	if (kind == Or)
		ss << " or ";
	else if (kind == And)
		ss << " and ";
	ss << "(" << children[1]->str(ind) << ")";
	return ss.str();
}
CompareExp::~CompareExp () {}
std::string CompareExp::str (int ind) const
{
	std::ostringstream ss;
	ss << children[0]->str(ind);
	switch (kind)
	{
	case Lt:    ss << " < "; break;
	case LtEq:  ss << " <= "; break;
	case Eq:    ss << " == "; break;
	case NotEq: ss << " != "; break;
	case GrEq:  ss << " >= "; break;
	case Gr:    ss << " > "; break;
	default: break;
	}
	ss << children[1]->str(ind);
	return ss.str();
}
ConsExp::~ConsExp () {}
std::string ConsExp::str (int ind) const
{
	std::ostringstream ss;
	ss << children[0]->str(ind)
	   << " $ " << children[1]->str(ind);
	return ss.str();
}
NilExp::~NilExp () {}
std::string NilExp::str (int ind) const { return "[]"; }
FieldExp::~FieldExp () {}
std::string FieldExp::str (int ind) const
{
	return children[0]->str(ind) + "." + name;
}
MemberExp::~MemberExp () {}
std::string MemberExp::str (int ind) const
{
	std::ostringstream ss;
	ss << children[0]->str(ind) << "["
	   << children[1]->str(ind + 2) << "]";
	return ss.str();
}
CallExp::~CallExp () {}
std::string CallExp::str (int ind) const
{
	std::ostringstream ss;
	ss << children[0]->str(ind) << "(";
	for (size_t i = 1, len = children.size(); i < len; i++)
	{
		if (i > 1)
			ss << ", ";
		ss << children[i]->str(ind + 2);
	}
	ss << ")";
	return ss.str();
}
BlockExp::~BlockExp () {}
std::string BlockExp::str (int ind) const
{
	std::ostringstream ss;
	ss << "{" << std::endl;
	for (auto& e : children)
	{
		ss << spaces(ind + 2)
		   << e->str(ind + 2)
		   << std::endl;
	}
	if (unitResult)
		ss << spaces(ind + 2) << "()" << std::endl;
	ss << spaces(ind) << "}";
	return ss.str();
}
WhileExp::~WhileExp () {}
AssignExp::~AssignExp () {}
std::string AssignExp::str (int ind) const
{
	std::ostringstream ss;
	ss << children[0]->str(ind)
	   << " = " << children[1]->str(ind);
	return ss.str();
}
ReturnExp::~ReturnExp () {}
std::string ReturnExp::str (int ind) const
{
	return "return " + children[0]->str(ind + 2);
}
GotoExp::~GotoExp () {}
std::string GotoExp::str (int ind) const
{
	if (kind == Break)
		return "break";
	else
		return "continue";
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
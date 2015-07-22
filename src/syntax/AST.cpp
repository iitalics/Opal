#include <syntax/AST.h>

namespace AST
{

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

Type::~Type () {}
ParamType::~ParamType () {}
ConcreteType::~ConcreteType () {}

Func::~Func () {}
void Func::setImpl (const std::string& name, TypePtr type)
{
	impl = FuncArg { name, type };
}
Constant::~Constant () {}
IFace::~IFace () {}
void IFace::add (const std::string& name,
		FuncArgs args, TypePtr ret)
{
	funcs.push_back({ name, args, ret });
}

Toplevel::Toplevel () {}
Toplevel::~Toplevel () {}

}
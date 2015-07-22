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

FuncDecl::~FuncDecl ();
TypeDecl::~TypeDecl ();
ConstDecl::~ConstDecl ();
IFaceDecl::~IFaceDecl ();
void IFaceDecl::add (const IFaceFunc& func)
{
	funcs.push_back(func);
}

Toplevel::Toplevel () {}
Toplevel::~Toplevel () {}

}
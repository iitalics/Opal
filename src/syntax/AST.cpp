#include <syntax/AST.h>

namespace AST
{
Exp::~Exp () {}
Type::~Type () {}
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
FieldExp::~FieldExp () {}
MemberExp::~MemberExp () {}
CallExp::~CallExp () {}
BlockExp::~BlockExp () {}
WhileExp::~WhileExp () {}
AssignExp::~AssignExp () {}
ReturnExp::~ReturnExp () {}
GotoExp::~GotoExp () {}

ParamType::~ParamType () {}
ConcreteType::~ConcreteType () {}

}
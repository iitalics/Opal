#pragma once
#include <syntax/AST.h>
#include <syntax/Scanner.h>

namespace Parse {
;
AST::Toplevel parseToplevel (Scanner& scan);
AST::ExpPtr parseExp (Scanner& scan);
AST::TypePtr parseType (Scanner& scan);
}
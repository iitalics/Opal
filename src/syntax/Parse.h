#pragma once
#include "AST.h"
#include "Scanner.h"
namespace Opal { namespace Parse {
;

AST::Toplevel parseToplevel (Scanner& scan);
AST::ExpPtr parseExp (Scanner& scan);
AST::TypePtr parseType (Scanner& scan);


}}

#pragma once
#include "AST.h"
namespace Opal { namespace Desugar {
;

void desugar (AST::TypePtr& ty);
void desugar (AST::ExpPtr& e);
void desugar (AST::PatPtr& p);
void desugar (AST::DeclPtr& decl);


}}

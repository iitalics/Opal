#pragma once
#include "Env.h"
namespace Opal { namespace Env {
;


class Namespace
{
public:
	static inline Namespace* all () { return _all; }
	inline Namespace* next () const { return _next; }

	Namespace (Module* priv, Module* publ);
	~Namespace ();

	Module* modPrivate;
	Module* modPublic;
	std::set<Module*> imports;
	std::vector<AST::DeclPtr> decls;

protected:
	Namespace* _next;
	static Namespace* _all;
};


}}

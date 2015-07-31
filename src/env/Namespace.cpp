#include "Namespace.h"
namespace Opal { namespace Env {
;

Namespace* Namespace::_all = nullptr;

Namespace::Namespace (Module* priv, Module* publ)
	: modPrivate(priv), modPublic(publ), _next(_all)
{
	_all = this;
	imports.insert(priv);
	imports.insert(publ);
}
Namespace::~Namespace ()
{
	if (_all == this)
		_all = _next;
}


Type* Namespace::getType (const AST::Name& name) const
{
	if (name.hasModule())
		return Module::get(name.module)->getType(name.name);
	else
		for (auto& imp : imports)
			if (auto r = imp->getType(name.name))
				return r;
	return nullptr;
}


}}

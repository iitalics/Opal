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


}}

#include "Loader.h"
namespace Opal { namespace Env {
;


PackageLoad* PackageLoad::_reqs = nullptr;
std::map<std::string, Package*> Package::_pkgs;


Package::Package (const std::string& n)
	: _name(n)
{
	// add to list
	if (_pkgs[n])
		throw SourceError("duplicate package: " + n);

	_pkgs[n] = this;
}
Package::~Package () {}

Package& Package::byName (const std::string& name, const Span& sp)
{
	// retrieve from dictionary
	auto it = _pkgs.find(name);
	if (it == _pkgs.end())
		throw SourceError("package does not exist: " + name, sp);
	else
		return *(it->second);
}


PackageLoad::PackageLoad (const std::string& name, Handler handler)
	: prev(nullptr), next(_reqs), _name(name), _handler(handler)
{ _reqs = this; }
PackageLoad::~PackageLoad ()
{
	// remove from double linked list
	PackageLoad* prev = nullptr;
	for (auto r = _reqs; r != nullptr; prev = r, r = r->next)
	{
		if (r != this)
			continue;

		r->next->prev = prev;
		if (prev == nullptr)
			_reqs = r->next;
		else
			prev->next = r->next;
		break;
	}
}
void PackageLoad::finish ()
{
	while (_reqs != nullptr)
	{
		auto r = _reqs;
		_reqs = r->next;

		// call the loader code
		r->_handler(*(new Package(r->_name)));
	}
}


}}

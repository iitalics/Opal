#pragma once
#include "Env.h"
#include "Namespace.h"
namespace Opal { namespace Env {
;



struct Package
{
	// find a package
	static Package& byName (const std::string& name,
		const Span& sp = Span());

	// package name
	inline std::string name () const
	{
		return _name;
	}

	// add or retrieve data from the package
	template <typename T>
	Package& put (const std::string& key, T data)
	{
		_contents.insert({ key, (void*) data });
		return *this;
	}
	template <typename T>
	T get (const std::string& key)
	{
		return (T) _contents[key];
	}

	// private constructor
	~Package ();
protected:
	// internals
	static std::map<std::string, Package*> _pkgs;

	friend struct PackageLoad;
	Package (const std::string& name);

	std::string _name;
	std::map<std::string, void*> _contents;
};

// structure to abuse C++'s static
//  initialization before main()
// usage: create a Handler function that loads
//  the data into a package, then create
//  a static PackageLoad object, which tells
//  Opal to request that this new package be made
/* e.g.
	void loadMyPackage (Env::Package& pkg)
	{
		// loading code
		pkg.put(..., ...);
	}

	// package request
	Env::PackageLoad _("my.package", loadMyPackage);
*/
struct PackageLoad
{
	using Handler = void (*) (Package&);
	PackageLoad (const std::string& name,
		Handler handler);
	~PackageLoad ();

	static void finish ();
private:
	static PackageLoad* _reqs;

	PackageLoad* prev, * next;
	std::string _name;
	Handler _handler;
};



static std::set<std::string> searchPaths;


Module* loadModule (const std::string& name);
Namespace* loadSource (const std::string& path);

void finishModuleLoad ();

}}

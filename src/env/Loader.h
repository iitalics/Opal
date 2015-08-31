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
		auto it = _contents.find(key);
		if (it == _contents.end())
			return (T) nullptr;
		else
			return (T) it->second;
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
	static void loadMyPackage (Env::Package& pkg)
	{
		// loading code
		pkg.put(..., ...);
	}

	// package request
	static Env::PackageLoad _("my.package", loadMyPackage);
*/
struct PackageLoad
{
	using Handler = void (*) (Package&);
	PackageLoad (const std::string& name,
		Handler handler,
		const std::vector<std::string>& mods = {});
	~PackageLoad ();

	static void moduleLoad ();
	static void finish ();
private:
	static PackageLoad* _reqs;

	PackageLoad* prev, * next;
	std::string _name;
	Handler _handler;
	std::vector<std::string> _mods;
};



// search paths stuff
extern std::set<std::string> searchPaths;
void initSearchPaths (const std::string& prgm);

Module* loadModule (const std::string& name);
Namespace* loadSource (const std::string& path);

void finishModuleLoad ();

}}

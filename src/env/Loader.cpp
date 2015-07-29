#include "Loader.h"
#include "../syntax/Parse.h"
#include "../syntax/Desugar.h"
namespace Opal { namespace Env {
;


Module* loadModule (const std::string& name)
{
	auto mod = Module::get(name);
	if (mod->loaded)
		return mod;;
	mod->loaded = true;

	std::cout << "requested module '" << name << "'" << std::endl;

	return mod;
}

void loadSource (const std::string& path)
{
	Scanner scan(path);
	auto toplevel = Parse::parseToplevel(scan);

	Module* mod;
	if (toplevel.module.empty())
		mod = Module::make();
	else
		mod = loadModule(toplevel.module);

	mod->decls.reserve(mod->decls.size() + toplevel.decls.size());

	for (auto& name : toplevel.uses)
	{
		mod->importing.push_back(loadModule(name));
	}
	for (auto decl : toplevel.decls)
		Desugar::desugar(decl);
}


void finishModuleLoad ()
{
	// declare Types from TypeDecls and IFaceDecls
	//   (check for duplicates)
	// import Types
	// create Types and Globals
	//   (check for duplicates)
	//   (check that referenced types exist and are used properly)
	// import Globals
	// infer functions
	// generate code
	// find entry point and execute
}


}}

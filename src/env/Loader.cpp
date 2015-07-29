#include "Loader.h"
#include "../syntax/Parse.h"
#include "../syntax/Desugar.h"
namespace Opal { namespace Env {
;


static SourceError DupError (const std::string& kind,
		const std::string& name, const Span& s1, const Span& s2)
{
	std::ostringstream ss;
	ss << "duplicate definition of " << kind << " '" << name << "'";
	return SourceError(ss.str(), { s1, s2 });
}




Module* loadModule (const std::string& name)
{
	auto mod = Module::get(name);
	if (mod->loaded)
		return mod;
	mod->loaded = true;

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

	for (auto& name : toplevel.uses)
		mod->importing.push_back(loadModule(name));
	for (auto decl : toplevel.decls)
	{
		Desugar::desugar(decl);
		mod->decls.push_back(decl);
	}
}





static void declareType (Module* mod, AST::DeclPtr decl)
{
	std::string name;
	Span span;
	size_t nparams;
	bool isIFace;

	if (auto tydecl = dynamic_cast<AST::TypeDecl*>(decl.get()))
	{
		name = tydecl->name;
		span = tydecl->span;
		nparams = tydecl->args.size();
		isIFace = false;
	}
	else if (auto iface = dynamic_cast<AST::IFaceDecl*>(decl.get()))
	{
		name = iface->name;
		span = iface->span;
		nparams = 0;
		isIFace = true;
	}
	else
		return;

	auto ty = mod->getType(name);
	if (ty != nullptr)
		throw DupError("type", ty->name, ty->declSpan, span);

	// ++ memory allocated here ++
	auto res = new Type {
		name,
		mod,
		span,
		isIFace
	};
	res->data.nparams = nparams;
	mod->types.push_back(res);

	std::cout << "declared type " << name << std::endl;
}
static void declareTypes ()
{
	for (auto mod = Module::all(); mod != nullptr; mod = mod->next())
	{
		for (auto& decl : mod->decls)
			declareType(mod, decl);
	}
}

static void importTypes (Module* dest, Module* src)
{
	for (auto& ty : src->types)
		if (ty->module == src)
			dest->types.push_back(ty);
		else
			break;
}
static void importTypes ()
{
	for (auto mod = Module::all(); mod != nullptr; mod = mod->next())
		for (auto& use : mod->importing)
			importTypes(mod, use);
}

void finishModuleLoad ()
{
	// declare Types from TypeDecls and IFaceDecls
	//   (check for duplicates)
	declareTypes();
	// import Types
	importTypes();
	// create Types and Globals
	//   (check for duplicates)
	//   (check that referenced types exist and are used properly)
	// import Globals
	// infer functions
	// generate code
	// find entry point and execute
}


}}

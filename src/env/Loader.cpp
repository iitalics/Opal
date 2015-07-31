#include "Loader.h"
#include "Namespace.h"
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

	// search through paths to find sources
	//  for module

	return mod;
}

Namespace* loadSource (const std::string& path)
{
	Scanner scan(path);
	auto toplevel = Parse::parseToplevel(scan);

	Module* mod_priv = new Module();
	Module* mod_pub;

	if (toplevel.module.empty())
		mod_pub = mod_priv;
	else
		mod_pub = loadModule(toplevel.module);

	auto nm = new Namespace(mod_priv, mod_pub);

	for (auto& name : toplevel.uses)
		nm->imports.insert(loadModule(name));

	for (auto decl : toplevel.decls)
		Desugar::desugar(decl);

	nm->decls = toplevel.decls;
	return nm;
}





static void declareType (Namespace* nm, AST::DeclPtr decl)
{
	auto mod = decl->isPublic ? nm->modPublic : nm->modPrivate;

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

	auto type = nm->getType(name);
	if (type != nullptr)
		throw DupError("type", type->name, type->declSpan, span);

	// ++ memory allocated here ++
	auto res = new Type {
		name,
		mod,
		span,
		isIFace
	};
	res->data.nparams = nparams;
	mod->types.push_back(res);

	std::cout << "declared type " << res->fullname().str() << std::endl;
}
static void declareTypes ()
{
	for (auto nm = Namespace::all(); nm != nullptr; nm = nm->next())
		for (auto decl : nm->decls)
			declareType(nm, decl);
}




static void createType (Namespace* nm, Module* mod, AST::TypeDecl* tydecl)
{
	auto type = mod->getType(tydecl->name);

	Infer::Type::Ctx ctx(nm);

	// find parameters
	for (auto& arg : tydecl->args)
	{
		size_t expectedId = ctx.params.size();
		auto ty = Infer::Type::fromAST(arg, ctx);

		if (ty->id != expectedId)
			throw SourceError("duplicate parameter names",
				{ ctx.spans[ty->id], arg->span });
	}

	// new parameters may not be declared in fields
	ctx.allowNewTypes = false;

	size_t nfields = tydecl->fields.size();
	type->data.nfields = nfields;
	type->data.fields = new Infer::Var[nfields];

	// create fields
	for (size_t i = 0; i < nfields; i++)
	{
		type->data.fields[i] =
			Infer::Var::fromAST(tydecl->fields[i], ctx);

	//	std::cout << type->data.fields[i].type->str() << std::endl;
	}
	std::cout << "created type " << type->fullname().str() << std::endl;
}
static void create (Namespace* nm, AST::DeclPtr decl)
{
	auto mod = decl->isPublic ? nm->modPublic : nm->modPrivate;

	if (auto tydecl = dynamic_cast<AST::TypeDecl*>(decl.get()))
	{
		createType(nm, mod, tydecl);
	}
}
static void createAll ()
{
	for (auto nm = Namespace::all(); nm != nullptr; nm = nm->next())
		for (auto decl : nm->decls)
			create(nm, decl);
}


void finishModuleLoad ()
{
	// declare Types from TypeDecls and IFaceDecls
	//   (check for duplicates)
	declareTypes();
	// create Types and Globals
	//   (check for duplicate globals/methods)
	//   (check that referenced types exist and are used properly)
	createAll();
	// infer functions
	// generate code
	// find entry point and execute
}


}}

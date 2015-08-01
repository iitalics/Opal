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
		nparams = iface->args.size();
		isIFace = true;
	}
	else
		return;

	// duplicate?
	auto type = nm->getType(name);
	if (type != nullptr)
		throw DupError("type", type->name, type->declSpan, span);

	// ++ memory allocated here ++
	auto res = new Type {
		name,
		mod,
		span,
		nparams,
		isIFace
	};
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

	// make parameters
	for (auto& arg : tydecl->args)
		ctx.createParam(arg, {});

	// new parameters may not be declared in fields
	ctx.allowNewTypes = false;

	size_t nfields = tydecl->fields.size();
	type->data.nfields = nfields;
	type->data.fields = new Infer::Var[nfields];

	// create fields from AST
	for (size_t i = 0; i < nfields; i++)
	{
		type->data.fields[i] =
			Infer::Var::fromAST(tydecl->fields[i], ctx);
	}
	std::cout << "created type " << type->fullname().str() << std::endl;
}
static void createIFace (Namespace* nm, Module* mod, AST::IFaceDecl* ifdecl)
{
	auto type = mod->getType(ifdecl->name);

	Infer::Type::Ctx ctx(nm);

	// make parameters
	ctx.createParam(ifdecl->selfParam, {});
	for (auto& arg : ifdecl->args)
		ctx.createParam(arg, {});

	size_t nfuncs = ifdecl->funcs.size();
	type->iface.nfuncs = nfuncs;
	type->iface.funcs = new Env::IFaceSignature[nfuncs];

	// create methods
	for (size_t i = 0; i < nfuncs; i++)
	{
		// duplicate?
		for (size_t j = 0; j < i; j++)
			if (ifdecl->funcs[j].name == ifdecl->funcs[i].name)
				throw DupError("method", ifdecl->funcs[i].name,
					ifdecl->funcs[j].span,
					ifdecl->funcs[i].span);

		auto& fnsig = type->iface.funcs[i];
		auto& fn = ifdecl->funcs[i];

		// new context for each function
		Infer::Type::Ctx ctx2(ctx);

		// create arguments from AST
		fnsig.name = fn.name;
		fnsig.declSpan = fn.span;
		fnsig.argc = fn.args.size();
		fnsig.args = new Infer::TypePtr[fnsig.argc];
		for (size_t j = 0; j < fnsig.argc; j++)
			fnsig.args[j] = Infer::Type::fromAST(fn.args[j], ctx);

		fnsig.ret = Infer::Type::fromAST(fn.ret, ctx);
	}
	std::cout << "created iface " << type->fullname().str() << std::endl;
}
static void createFunc (Namespace* nm, Module* mod, AST::FuncDecl* fndecl)
{
	Global* global = nullptr;
	Infer::Var impl { "", nullptr };
	Type* implBase = nullptr;

	Infer::Type::Ctx ctx(nm);

	// make global, or verify base type in impl
	if (fndecl->impl.type == nullptr)
	{
		// duplicate?
		global = nm->getGlobal(fndecl->name);
		if (global != nullptr)
			throw DupError("global", fndecl->name,
				global->declSpan, fndecl->span);

		// ++ memory allocated here ++
		global = new Global {
			fndecl->name,
			mod,
			fndecl->span,
			true
		};
		mod->globals.push_back(global);
	}
	else
	{
		impl = Infer::Var::fromAST(fndecl->impl, ctx);

		if (impl.type->kind != Infer::Type::Concrete ||
				impl.type->base->isIFace)
			throw SourceError("methods may only belong to concrete, non-iface types",
				fndecl->impl.type->span);

		implBase = impl.type->base;

		// duplicate?
		for (auto& fn : implBase->methods)
			if (fn->name == fndecl->name)
				throw DupError("method", fndecl->name,
					fn->declSpan, fndecl->span);
	}

	// ++ memory allocated here ++
	auto fn = new Function {
		Function::CodeFunction, // TODO: external functions
		fndecl->name,
		mod,
		fndecl->span
	};
	fn->args.reserve(fndecl->args.size() + 1);
	fn->ret = nullptr; // not yet inferred
	fn->body = fndecl->body;

	if (implBase != nullptr)
		fn->args.push_back(impl);

	for (auto& arg : fndecl->args)
	{
		auto var = Infer::Var::fromAST(arg, ctx);
		fn->args.push_back(var);
	}

	if (global != nullptr)
	{
		global->func = fn;
		std::cout << "created global function " << global->fullname().str() << std::endl;
	}
	if (implBase != nullptr)
	{
		implBase->methods.push_back(fn);
		std::cout << "created method " << implBase->fullname().str() << "." << fn->name << std::endl;
	}
}
static void create (Namespace* nm, AST::DeclPtr decl)
{
	auto mod = decl->isPublic ? nm->modPublic : nm->modPrivate;

	if (auto tydecl = dynamic_cast<AST::TypeDecl*>(decl.get()))
	{
		createType(nm, mod, tydecl);
	}
	else if (auto ifdecl = dynamic_cast<AST::IFaceDecl*>(decl.get()))
	{
		createIFace(nm, mod, ifdecl);
	}
	else if (auto fndecl = dynamic_cast<AST::FuncDecl*>(decl.get()))
	{
		createFunc(nm, mod, fndecl);
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

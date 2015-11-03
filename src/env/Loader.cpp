#include "Loader.h"
#include "Namespace.h"
#include "../syntax/Parse.h"
#include "../syntax/Desugar.h"
#include "../code/CodeGen.h"
#include <cstdlib>
#include <functional>
#include <dirent.h>
namespace Opal { namespace Env {
;

#define SILENT_LOADER

#define LIBS_NAME "/opal_libs"
#define CORE_LIBS "/opal_core"

#define WIN_ETC "APPDATA"
#define UNIX_ETC "/etc"

static std::vector<std::string> validExts { ".opal" };


static SourceError DupError (const std::string& kind,
		const std::string& name, const Span& s1, const Span& s2)
{
	std::ostringstream ss;
	ss << "duplicate definition of " << kind << " '" << name << "'";
	return SourceError(ss.str(), { s1, s2 });
}




std::set<std::string> searchPaths;

void initSearchPaths (const std::string& prgm)
{
#ifdef WIN3
	std::string etc(std::getenv(WIN_ETC))
#else
	std::string etc(UNIX_ETC);
#endif

	searchPaths.insert(etc + CORE_LIBS);
	searchPaths.insert(".");

	for (int i = prgm.size(); i-- > 0; )
		if (prgm[i] == '/' || prgm[i] == '\\')
		{
			searchPaths.insert(prgm.substr(0, i));
			break;
		}
}
template <typename T>
static bool readDirectory (const std::string& path, T callback)
{
	// http://stackoverflow.com/questions/612097
	DIR* dir;
	struct dirent* ent;

	if ((dir = opendir(path.c_str())) != NULL)
	{
		while ((ent = readdir(dir)) != NULL)
			callback(std::string(ent->d_name));
		closedir(dir);

		return true;
	}
	else
		return false;
}



Module* loadModule (const std::string& moduleName)
{
	auto mod = Module::get(moduleName);
	if (mod->loaded)
		return mod;
	mod->loaded = true;

	for (auto path : searchPaths)
	{
		std::ostringstream ss;
		ss << path << LIBS_NAME << "/" << moduleName << "/";
		auto prefix = ss.str();

		bool success = readDirectory(prefix, [=] (std::string file)
		{
			for (auto ext : validExts)
				if (file.find(ext) != file.size() - ext.size())
					return;

			loadSource(prefix + file);
		});
		if (success) break;
	}

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
	auto res = new Type(
		name,
		mod,
		nparams,
		isIFace,
		span);
	mod->types.push_back(res);
}
static void declareTypes ()
{
	for (auto nm = Namespace::all(); nm != nullptr; nm = nm->next())
		for (auto decl : nm->decls)
			declareType(nm, decl);
}



static void createEnumFunc (AST::EnumFunc& efn, Module* mod,
	Infer::Type::Ctx& ctx, Infer::TypePtr ret)
{
	// create global function object
	auto fn = new Function(Function::EnumFunction,
		efn.name,
		mod,
		efn.span);
	auto global = new Global {
		efn.name,
		mod,
		efn.span,
		.isFunc = true,
		.func = fn
	};
	mod->globals.push_back(global);

	// get arguments
	for (auto& arg : efn.args)
	{
		auto ty = Infer::Type::fromAST(arg, ctx);
		fn->args.push_back({ "", ty });
	}
	fn->ret = ret;

	// runtime info
	fn->enumType = ret->base;
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
	if (tydecl->isExtern)
	{
		type->userCreate = false;
		type->gc_collected = tydecl->gcCollect;
	}
	else if (tydecl->isEnum)
		type->userCreate = false;
	else
		type->userCreate = true;

	// create fields from AST
	for (size_t i = 0; i < nfields; i++)
	{
		// duplicate?
		auto name = tydecl->fields[i].name;
		for (size_t j = 0; j < i; j++)
			if (name == type->data.fields[j].name)
				throw DupError("field", name,
					type->data.fields[j].declSpan,
					tydecl->fields[i].span);

		type->data.fields[i] =
			Infer::Var::fromAST(tydecl->fields[i], ctx);
	}

	// create enum functions
	if (tydecl->isEnum)
	{
		Infer::TypeList args;
		for (size_t i = 0; i < type->nparams; i++)
		{
			auto p = ctx.params[type->nparams - i - 1];
			args = Infer::TypeList(p, args);
		}
		auto ret = Infer::Type::concrete(type, args);

		for (auto& efn : tydecl->enumfns)
			createEnumFunc(efn, mod, ctx, ret);
	}
#ifndef SILENT_LOADER
	std::cout << "created type " << type->fullname().str() << std::endl;
#endif
}
static void createIFace (Namespace* nm, Module* mod, AST::IFaceDecl* ifdecl)
{
	auto type = mod->getType(ifdecl->name);

	Infer::Type::Ctx ctx(nm);

	// make parameters
	ctx.createParam(ifdecl->selfParam, {});
	for (auto& arg : ifdecl->args)
		ctx.createParam(arg, {});
	ctx.allowNewTypes = false;

	size_t nfuncs = ifdecl->funcs.size();
	type->iface.nfuncs = nfuncs;
	type->iface.funcs = new IFaceSignature[nfuncs];

	// create methods
	for (size_t i = 0; i < nfuncs; i++)
	{
		// duplicate?
		for (size_t j = 0; j < i; j++)
			if (ifdecl->funcs[j].name == ifdecl->funcs[i].name)
				throw DupError("method", ifdecl->funcs[i].name,
					ifdecl->funcs[j].span,
					ifdecl->funcs[i].span);

		auto& methoddecl = ifdecl->funcs[i];
		auto& fnsig = type->iface.funcs[i];

		// create method from AST
		fnsig.name = methoddecl.name;
		fnsig.declSpan = methoddecl.span;

		auto ret = Infer::Type::fromAST(methoddecl.ret, ctx);
		Infer::TypeList args(ret);
		for (auto it = methoddecl.args.rbegin(); it != methoddecl.args.rend(); ++it)
		{
			auto ty = Infer::Type::fromAST(*it, ctx);
			args = Infer::TypeList(ty, args);
		}
		fnsig.type = Infer::Type::concrete(
			Type::function(methoddecl.args.size()),
			args);

		// create callable method
		// ++ memory allocated here ++
		auto method = new Function(
			Function::IFaceFunction,
			fnsig.name,
			mod,
			ifdecl->span);
		method->nm = nm;
		method->ret = ret;
		method->ifaceSig = &fnsig;
		method->parent = type;
		type->methods.push_back(method);
	}
#ifndef SILENT_LOADER
	std::cout << "created iface " << type->fullname().str() << std::endl;
#endif
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

	bool ext = fndecl->isExtern;

	// ++ memory allocated here ++
	auto fn = new Function(
		ext ? Function::NativeFunction
		    : Function::CodeFunction,
		fndecl->name,
		mod,
		fndecl->span);
	fn->nm = nm;

	// external function (native)
	if (ext)
	{
		auto pkg = Package::byName(fndecl->pkg, fndecl->span);
		auto nativefn = pkg.get<Run::NativeFn_t>(fndecl->key);

		if (nativefn == nullptr)
		{
			std::ostringstream ss;
			ss << "undefined external function '" << fndecl->key << "'";
			throw SourceError(ss.str(), fndecl->span);
		}

		fn->nativeFunc = nativefn;
		fn->ret = Infer::Type::fromAST(fndecl->ret, ctx);
		fn->body = nullptr;
	}
	else // normal code function
		fn->body = fndecl->body;

	if (implBase != nullptr)
		fn->args.push_back(impl);

	// create arguments
	fn->args.reserve(fndecl->args.size() + 1);
	for (auto& arg : fndecl->args)
	{
		auto var = Infer::Var::fromAST(arg, ctx);
		fn->args.push_back(var);
	}

	// add to global scope, or make a method
	if (global != nullptr)
	{
		global->func = fn;
#ifndef SILENT_LOADER
		std::cout << "created global function " << global->fullname().str() << std::endl;
		if (!ext)
			std::cout << fn->body->str() << std::endl;
#endif
	}
	if (implBase != nullptr)
	{
		fn->parent = implBase;
		implBase->methods.push_back(fn);
#ifndef SILENT_LOADER
		std::cout << "created method " << implBase->fullname().str() << "." << fn->name << std::endl;
#endif
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

static void inferGlobal (Global* g)
{
	g->func->infer();
}
static void inferType (Type* ty)
{
	for (auto& fn : ty->methods)
		fn->infer();
}
static void inferAll ()
{
	for (auto mod = Module::all(); mod != nullptr; mod = mod->next())
	{
		for (auto g : mod->globals)
			inferGlobal(g);
		for (auto ty : mod->types)
			inferType(ty);
	}
}
static void codeGen (Function* func)
{
	if (func->kind == Function::CodeFunction)
		func->code = Code::CodeGen::generate(func);
}
static void codeGenAll ()
{
	for (auto mod = Module::all(); mod != nullptr; mod = mod->next())
	{
		for (auto g : mod->globals)
			codeGen(g->func);

		// triple-nested for loops
		// hard to pull off
		for (auto ty : mod->types)
			for (auto func : ty->methods)
				codeGen(func);
	}
}
static void releaseExpObjects (Function* func)
{
	if (func->kind == Function::CodeFunction)
		func->body = nullptr;
}
static void releaseExpObjects ()
{
	// no more declares
	for (auto nm = Namespace::all(); nm != nullptr; nm = nm->next())
		nm->decls.resize(0);

	for (auto mod = Module::all(); mod != nullptr; mod = mod->next())
	{
		// lambdas
		for (auto func : mod->lambdas())
			releaseExpObjects(func);

		// globals
		for (auto glob : mod->globals)
			releaseExpObjects(glob->func);

		// methods
		for (auto ty : mod->types)
			for (auto func : ty->methods)
				releaseExpObjects(func);
	}
}


void finishModuleLoad ()
{
	Module::getCore();
	PackageLoad::moduleLoad();
	// declare Types from TypeDecls and IFaceDecls
	//   (check for duplicates)
	declareTypes();
	// load external packages
	PackageLoad::finish();
	// create Types and Globals
	//   (check for duplicate globals/methods)
	//   (check that referenced types exist and are used properly)
	createAll();
	// infer functions
	Infer::Analysis::initTypes();
	inferAll();
	// generate code
	Run::Cell::initTypes();
	codeGenAll();

	// find and release Exp objects now that we don't
	//  need to use them anymore
	releaseExpObjects();
}


}}

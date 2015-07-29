#pragma once
#include "../opal.h"
#include "../infer/Type.h"
#include "../syntax/AST.h"

namespace Opal { namespace Env {

class Function;
class Type;
class Module;
class Global;

class Module
{
public:
	static Module* get (const std::string& name);
	static Module* make (const std::string& name);
	static Module* make ();
	static inline Module* all () { return _all; }
	inline Module* next () { return _next; }
	~Module ();

	std::string name;

	// types and variables have seperate namespaces
	std::vector<Type*> types;
	std::vector<Global*> globals;

	std::set<Module*> importing;
	std::vector<AST::DeclPtr> decls;

	Type* getType (const std::string& name) const;
	Type* getType (const AST::Name& name) const;
	Global* getGlobal (const std::string& name) const;
	Global* getGlobal (const AST::Name& name) const;

	bool loaded;
protected:
	Module (const std::string& _name);
	Module* _next;

	static Module* _all;
};

struct DataType
{
	std::string* params;
	size_t nparams;

	Infer::Var* fields;
	size_t nfields;

	void destroy ();
};

struct IFaceSignature
{
	~IFaceSignature ();

	std::string methodName;
	Infer::TypePtr* args;
	size_t argc;
	Infer::TypePtr ret;
};

struct IFaceType
{
	IFaceSignature* funcs;
	size_t nfuncs;

	void destroy ();
};

class Type
{
public:
	~Type ();
	std::string name;
	Module* module;
	Span declSpan;
	AST::Name fullname () const;

	bool isIFace;

	union
	{
		DataType data;
		IFaceType iface;
	};

	inline int args () const {
		if (isIFace)
			return 0;
		else
			return data.nparams;
	}

	std::vector<Function*> methods;
};

class Global
{
public:
	std::string name;
	Module* module;
	Span declSpan;
	AST::Name fullname () const;

	bool isFunc;
	Infer::TypePtr type;
	Function* func;
};

class Function
{
public:
	enum Kind
	{
		CodeFunction,    // code-defined function
		NativeFunction,  // native call function
		IFaceFunction,   // iface method function (does lookup)
		EnumFunction,    // enum constructor function
	};
	Kind kind;
	std::string name;
	Module* module;
	Span declSpan;
	AST::Name fullname () const;

	std::vector<Infer::Var> args;
	Infer::TypePtr ret;

	AST::ExpPtr body;
};


}}

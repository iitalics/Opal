#pragma once
#include "../opal.h"
#include "../infer/Type.h"
#include "../syntax/AST.h"
#include "../runtime/Exec.h"
namespace Opal {
namespace Infer {
class Analysis;
struct LocalEnv;
}
namespace Env {
;

class Function;
class Type;
class Module;
class Global;

class Module
{
public:
	static Module* get (const std::string& name);
	static inline Module* all () { return _all; }
	inline Module* next () const { return _next; }
	static Module* getCore ();

	Module (const std::string& _name);
	Module ();
	~Module ();

	std::string name;

	// types and variables have seperate namespaces
	std::vector<Type*> types;
	std::vector<Global*> globals;

	Type* getType (const std::string& name) const;
	Global* getGlobal (const std::string& name) const;
	Function* getFunction (const std::string& name) const;
	Function* getFunction (const std::string& type, const std::string& method) const;

	Function* makeLambda (const Span& span = Span());
	inline const std::vector<Function*> lambdas () const {
		return _lambdas;
	}

	bool loaded;
protected:
	static Module* _all;

	Module* _next;
	std::vector<Function*> _lambdas;
};

struct DataType
{
	Infer::Var* fields;
	size_t nfields;

	void destroy ();
};

struct IFaceSignature
{
	~IFaceSignature ();

	std::string name;
	Span declSpan;
	Infer::TypePtr* args;
	size_t argc;
	Infer::TypePtr ret;

	Infer::TypePtr getType ();
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
	// special types
	static Type* function (size_t argc);
	static Type* tuple (size_t argc);
	static Type* core (const std::string& name);

	// fields
	Type (const std::string& name,
		Module* mod, size_t nparams,
		bool iface, const Span& span = Span());
	~Type ();
	std::string name;
	Module* module;
	Span declSpan;
	AST::Name fullname () const;
	inline bool isFunction () const {
		return _function;
	}
	inline bool isTuple () const {
		return _tuple;
	}

	Function* getMethod (const std::string& name) const;

	// type data
	size_t nparams;
	bool isIFace;
	bool userCreate;
	union
	{
		DataType data;
		IFaceType iface;
	};
	std::vector<Function*> methods;


	bool gc_collected;
private:
	bool _function, _tuple;
};

class Global
{
public:
	~Global ();
	std::string name;
	Module* module;
	Span declSpan;
	AST::Name fullname () const;

	bool isFunc;
	Function* func;

	Infer::TypePtr getType ();
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
	Function (Kind kind, const std::string& name, Module* mod,
		const Span& span = Span());
	~Function ();
	Kind kind;
	std::string name;
	Module* module;
	Span declSpan;
	AST::Name fullname () const;

	Type* parent;
	std::vector<Infer::Var> args;
	Infer::TypePtr ret;

	Namespace* nm;
	AST::ExpPtr body;

	union
	{
		struct
		{
			Infer::Analysis* analysis;
			Run::Code* code;
			Infer::LocalEnv* localEnv;
		};
		Run::NativeFn_t nativeFunc;
		IFaceSignature* ifaceSig;
		Env::Type* enumType;
	};

	void infer (Infer::Analysis* calledBy = nullptr);
	void endInfer ();
	Infer::TypePtr getType ();

private:
	void _endInfer ();
};


}}

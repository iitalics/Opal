#pragma once
#include "../opal.h"
#include "../infer/Type.h"
#include "../syntax/AST.h"

namespace Opal {
namespace Infer {
class Analysis;
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

	bool loaded;
protected:
	Module* _next;
	static Module* _all;
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

	size_t nparams;
	bool isIFace;

	union
	{
		DataType data;
		IFaceType iface;
	};

	std::vector<Function*> methods;
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

	Infer::Analysis* analysis;

	void infer (Infer::Analysis* calledBy = nullptr);
	void endInfer ();
	Infer::TypePtr getType ();

private:
	void _endInfer ();
};


}}

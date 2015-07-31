#pragma once
#include "../opal.h"
#include "../list.h"
#include "../syntax/AST.h"
namespace Opal {
namespace Env {
;
class Type;
class Global;
class Function;
class Namespace;
}

namespace Infer {
;

struct Type;
using TypePtr = std::shared_ptr<Type>;


struct Var
{
	std::string name;
	TypePtr type;
};

struct Type
{
	enum Kind { Concrete, Param, Poly };
	struct Ctx
	{
		inline explicit Ctx (Env::Namespace* _nm = nullptr)
			: nm(_nm), allowNewTypes(true) {}

		Env::Namespace* nm;
		bool allowNewTypes;
		std::vector<TypePtr> params;
		std::vector<Span> spans;
	};

	// convert AST to Type
	static TypePtr fromAST (AST::TypePtr ty, Ctx& ctx);

	// construct type
	static TypePtr concrete (Env::Type* base,
	                           const list<TypePtr>& args);
	static TypePtr param (int id, const std::string& name,
	                           const list<Env::Type*>& ifaces =
	                               list<Env::Type*>());
	static TypePtr poly (int id, const list<Env::Type*>& ifaces =
	                          list<Env::Type*>());
	
	inline Type (Kind _kind)
		: kind(_kind) {}
	~Type ();

	Kind kind;
	list<TypePtr> args;
	std::string paramName;
	list<Env::Type*> ifaces;
	union {
		Env::Type* base;
		int id;
	};

	std::string str () const;
	void set (TypePtr other);
};


}}

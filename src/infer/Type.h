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
using TypeList = list<TypePtr>;


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

		TypePtr createParam (const std::string& name, const TypeList& ifaces, const Span& sp = Span());
	};

	// convert AST to Type
	static TypePtr fromAST (AST::TypePtr ty, Ctx& ctx);

	// construct type
	static TypePtr concrete (Env::Type* base,
	                           const TypeList& args);
	static TypePtr param (int id, const std::string& name,
	                           const TypeList& ifaces = {});
	static TypePtr poly (int id, const TypeList& ifaces = {});
	
	inline Type (Kind _kind)
		: kind(_kind) {}
	~Type ();

	Kind kind;
	TypeList args;
	std::string paramName;
	union {
		Env::Type* base;
		int id;
	};

	std::string str () const;
	void set (TypePtr other);

	bool containsParam (const std::string& name);
	bool containsParam (int id);
};


struct Var
{
	std::string name;
	TypePtr type;

	static inline Var fromAST (AST::Var var, Type::Ctx& ctx) {
		return Var {
			var.name,
			Type::fromAST(var.type, ctx)
		};
	}
};



}}

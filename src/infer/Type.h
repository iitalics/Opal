#pragma once
#include "../opal.h"
#include "../list.h"
namespace Opal {
namespace Env {
;
class Type;
class Function;
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

struct Parameter
{
	std::string name;
};

struct Type
{
	enum Kind { Concrete, Param, Poly };
	Kind kind;

	Type (Kind _kind);
	~Type ();

	union
	{
		struct {
			Env::Type* base;
			TypePtr* args;
			size_t argc;
		} concrete;

		struct {
			Parameter* param;
			Env::Type** ifaces;
			size_t nifaces;
		} param;
	};

	std::string str () const;
	void set (TypePtr other);
};


}}

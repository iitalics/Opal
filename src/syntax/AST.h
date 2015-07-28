#pragma once
#include "Span.h"
#include <list.h>
namespace Opal { namespace AST {
;


class Exp;
class Type;
class Decl;
using ExpPtr = std::shared_ptr<Exp>;
using ExpList = std::vector<ExpPtr>;
using TypePtr = std::shared_ptr<Type>;
using TypeList = std::vector<TypePtr>;
using DeclPtr = std::shared_ptr<Decl>;

struct Name
{
	std::string name;
	std::string module;

	inline Name (const std::string& _name,
			const std::string& _module = "")
		: name(_name), module(_module) {}

	bool hasModule () const;
	std::string str () const;
};

class Exp
{
public:
	explicit inline Exp (const ExpList& _children = {})
		: children(_children) {}
	virtual ~Exp () = 0;
	virtual std::string str (int ident) const;
	inline std::string str () const { return str(0); }

	template <typename T>
	inline bool is () const {
		return dynamic_cast<const T*>(this) != nullptr;
	}

	ExpList children;
	Span span;
};

class VarExp : public Exp
{
public:
	Name name;
	bool global;
	int varID;

	explicit inline VarExp (const Name& _name) : name(_name) {}
	virtual ~VarExp ();
	virtual std::string str (int ident) const;
};
class IntExp : public Exp
{
public:
	Int_t value;
	explicit inline IntExp (Int_t _value) : value(_value) {}
	virtual ~IntExp();
	virtual std::string str (int ident) const;
};
class RealExp : public Exp
{
public:
	Real_t value;
	explicit inline RealExp (Real_t _value) : value(_value) {}
	virtual ~RealExp ();
	virtual std::string str (int ident) const;
};
class StringExp : public Exp
{
public:
	std::string value;
	explicit inline StringExp (const std::string& _value) : value(_value) {}
	virtual ~StringExp ();
	virtual std::string str (int ident) const;
};
class BoolExp : public Exp
{
public:
	bool value;
	explicit inline BoolExp (bool _value) : value(_value) {}
	virtual ~BoolExp ();
	virtual std::string str (int ident) const;
};
class TupleExp : public Exp
{
public:
	explicit inline TupleExp (const ExpList& ch) : Exp(ch) {}
	virtual ~TupleExp ();
	virtual std::string str (int ident) const;
};
class ListExp : public Exp
{
public:
	explicit inline ListExp (const ExpList& ch) : Exp(ch) {}
	virtual ~ListExp ();
	virtual std::string str (int ident) const;
};
class LambdaExp : public Exp
{
public:
	std::vector<std::string> args;

	inline LambdaExp (
			const std::vector<std::string>& _args,
			ExpPtr body)
		: Exp({ body }),
		  args(_args) {}
	virtual ~LambdaExp ();
	virtual std::string str (int ident) const;
};
class ObjectExp : public Exp
{
public:
	using Init = std::pair<std::string, ExpPtr>;

	TypePtr objType;
	std::vector<std::string> inits;

	ObjectExp (
		TypePtr _objType,
		const std::vector<Init>& _inits);
	virtual ~ObjectExp ();
	virtual std::string str (int ident) const;
};
class CondExp : public Exp
{
public:
	inline CondExp (ExpPtr _cond, ExpPtr _then, ExpPtr _else)
		: Exp({ _cond, _then, _else }) {}
	inline CondExp (ExpPtr _cond, ExpPtr _then)
		: Exp({ _cond, _then }) {}
	virtual ~CondExp ();
	virtual std::string str (int ident) const;
};
class LetExp : public Exp
{
public:
	std::string name;
	TypePtr varType;
	int varID;

	inline LetExp (const std::string& _name, ExpPtr _init)
		: Exp({ _init }), name(_name), varType(nullptr) {}
	inline LetExp (const std::string& _name, TypePtr _type)
		: name(_name), varType(_type) {}
	virtual ~LetExp ();
	virtual std::string str (int ident) const;
};
class LazyOpExp : public Exp
{
public:
	enum Kind { Or, And };

	Kind kind;
	inline LazyOpExp (ExpPtr _a, Kind _kind, ExpPtr _b)
		: Exp({ _a, _b }), kind(_kind) {}
	virtual ~LazyOpExp ();
	virtual std::string str (int ident) const;
};
class CompareExp : public Exp
{
public:
	enum Kind { Lt, LtEq, Eq, NotEq, Gr, GrEq };

	Kind kind;
	inline CompareExp (ExpPtr _a, Kind _kind, ExpPtr _b) // sugar
		: Exp({ _a, _b }), kind(_kind) {}
	virtual ~CompareExp ();
	virtual std::string str (int ident) const;
};
class ConsExp : public Exp
{
public:
	inline ConsExp (ExpPtr _hd, ExpPtr _tl)
		: Exp({ _hd, _tl }) {}
	virtual ~ConsExp ();
	virtual std::string str (int ident) const;
};
class NilExp : public Exp
{
public:
	virtual ~NilExp ();
	virtual std::string str (int ident) const;
};
class FieldExp : public Exp
{
public:
	std::string name;
	inline FieldExp (ExpPtr _a, const std::string& _name)
		: Exp({ _a }), name(_name) {}
	virtual ~FieldExp ();
	virtual std::string str (int ident) const;
};
class MemberExp : public Exp
{
public:
	inline MemberExp (ExpPtr _a, ExpPtr _mem)
		: Exp({ _a, _mem }) {}
	virtual ~MemberExp ();
	virtual std::string str (int ident) const;
};
class CallExp : public Exp
{
public:
	inline CallExp (ExpPtr _fn, const ExpList& _args)
		: Exp({ _fn })
	{
		children.insert(children.begin() + 1,
			_args.begin(), _args.end());
	}
	virtual ~CallExp ();
	virtual std::string str (int ident) const;
};
class BlockExp : public Exp
{
public:
	bool unitResult;
	inline BlockExp (const ExpList& stmts, bool _unitResult)
		: Exp(stmts), unitResult(_unitResult) {}
	virtual ~BlockExp ();
	virtual std::string str (int ident) const;
};
class WhileExp : public Exp
{
public:
	inline WhileExp (ExpPtr _cond, ExpPtr _body)
		: Exp({ _cond, _body }) {}
	virtual ~WhileExp ();
};
class AssignExp : public Exp
{
public:
	inline AssignExp (ExpPtr _lh, ExpPtr _rh)
		: Exp({ _lh, _rh }) {}
	virtual ~AssignExp ();
	virtual std::string str (int ident) const;
};
class ReturnExp : public Exp
{
public:
	explicit inline ReturnExp (ExpPtr _what)
		: Exp({ _what }) {}
	inline ReturnExp () {}
	virtual ~ReturnExp ();
	virtual std::string str (int ident) const;
};
class GotoExp : public Exp
{
public:
	enum Kind { Break, Continue };
	Kind kind;
	explicit inline GotoExp (Kind _kind)
		: kind(_kind) {}
	virtual ~GotoExp ();
	virtual std::string str (int ident) const;
};



class Type
{
public:
	Type ();
	virtual ~Type () = 0;

	Span span;

	template <typename T>
	inline bool is () const {
		return dynamic_cast<const T*>(this) != nullptr;
	}

	virtual std::string str () const;
};
class ParamType : public Type
{
public:
	std::string name;
	std::vector<Name> ifaces;
	inline ParamType (const std::string& _name,
			const std::vector<Name>& _ifaces)
		: name(_name), ifaces(_ifaces) {}
	virtual ~ParamType ();

	virtual std::string str () const;
};
class ConcreteType : public Type
{
public:
	Name name;
	TypeList subtypes;
	inline ConcreteType (const Name& _name,
			const TypeList& _subtypes)
		: name(_name), subtypes(_subtypes) {}
	virtual ~ConcreteType ();

	virtual std::string str () const;
};



struct Var
{
	std::string name;
	TypePtr type;
};

class Decl
{
public:
	virtual ~Decl () = 0;

	template <typename T>
	inline bool is () const {
		return dynamic_cast<const T*>(this) != nullptr;
	}
};

class FuncDecl : public Decl
{
public:
	std::string name;
	std::vector<Var> args;
	ExpPtr body;
	Var impl;

	Span span;

	inline FuncDecl (const std::string& _name, 
			const std::vector<Var>& _args, 
			ExpPtr _body)
		: name(_name), args(_args),
		  body(_body), impl { "", nullptr } {}
	virtual ~FuncDecl ();
};

class TypeDecl : public Decl
{
public:
	std::string name;
	TypeList args;

	TypePtr alias;
	std::vector<Var> members;

	Span span;

	inline TypeDecl (const std::string& _name,
			const TypeList& _args,
			TypePtr _alias)
		: name(_name), args(_args), alias(_alias) {}
	inline TypeDecl (const std::string& _name,
			const TypeList& _args,
			const std::vector<Var>& _members)
		: name(_name), args(_args), members(_members) {}
	virtual ~TypeDecl ();
};

class ConstDecl : public Decl
{
public:
	std::string name;
	TypePtr type;
	ExpPtr init;

	Span span;

	inline ConstDecl (const std::string& _name,
			TypePtr _type)
		: name(_name), type(_type), init(nullptr) {}
	inline ConstDecl (const std::string& _name,
			ExpPtr _init)
		: name(_name), type(nullptr), init(_init) {}
	virtual ~ConstDecl ();
};

struct IFaceFunc
{
	std::string name;
	std::vector<Var> args;
	TypePtr ret;
	Span span;
};

class IFaceDecl : public Decl
{
public:
	std::string name;
	std::string selfParam;
	std::vector<IFaceFunc> funcs;

	Span span;

	IFaceDecl (const std::string& _name, 
			const std::string& _self)
		: name(_name), selfParam(_self) {}
	virtual ~IFaceDecl ();

	void add (const IFaceFunc& func);
};


class Toplevel
{
public:
	std::string module;
	std::vector<std::string> uses;
	std::vector<DeclPtr> decls;

	Toplevel ();
	~Toplevel ();
};


}}

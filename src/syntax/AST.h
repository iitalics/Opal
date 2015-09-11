#pragma once
#include "Span.h"
#include <list.h>
namespace Opal {
namespace Env {
class Type;
class Function;
class Global;
} namespace Infer {
struct LocalVar;
struct LocalEnv;
};
namespace AST {
;


class Exp;  // expression
class Pat;  // pattern
class Type; // type
class Decl; // declaration
// everything is ref counted
using ExpPtr = std::shared_ptr<Exp>;
using PatPtr = std::shared_ptr<Pat>;
using ExpList = std::vector<ExpPtr>;
using TypePtr = std::shared_ptr<Type>;
using TypeList = std::vector<TypePtr>;
using DeclPtr = std::shared_ptr<Decl>;


struct Var
{
	std::string name;
	TypePtr type;
	Span span;
};

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

	template <typename T>
	inline bool is () const {
		return dynamic_cast<const T*>(this) != nullptr;
	}

	ExpList children;
	Span span;

	static ExpPtr cons (Span sp, ExpPtr hd, ExpPtr tl);
	static ExpPtr nil (Span sp);
};

class VarExp : public Exp
{
public:
	Name name;
	Infer::LocalVar* var;
	Env::Global* global;

	explicit inline VarExp (const Name& _name)
		: name(_name), var(nullptr), global(nullptr) {}
	virtual ~VarExp ();
};
class NumberExp : public Exp
{
public:
	enum Kind { Int, Real, Long };

	Kind kind;
	union {
		Int_t intValue;
		Real_t realValue;
		Long_t longValue;
	};

	void castReal ();
	void castLong ();
	void negate ();

	explicit inline NumberExp (Int_t v)
		: kind(Int), intValue(v) {}
	explicit inline NumberExp (Real_t v)
		: kind(Real), realValue(v) {}
	explicit inline NumberExp (Long_t v)
		: kind(Long), longValue(v) {}
	virtual ~NumberExp ();
};
class StringExp : public Exp
{
public:
	std::string value;
	explicit inline StringExp (const std::string& _value) : value(_value) {}
	virtual ~StringExp ();
};
class BoolExp : public Exp
{
public:
	bool value;
	explicit inline BoolExp (bool _value) : value(_value) {}
	virtual ~BoolExp ();
};
class CharExp : public Exp
{
public:
	Char_t value;
	explicit inline CharExp (Char_t _value)
		: value(_value) {}
	virtual ~CharExp ();
};
class TupleExp : public Exp
{
public:
	explicit inline TupleExp (const ExpList& ch) : Exp(ch) {}
	virtual ~TupleExp ();
};
class ListExp : public Exp
{
public:
	explicit inline ListExp (const ExpList& ch) : Exp(ch) {}
	virtual ~ListExp ();
};
class LambdaExp : public Exp
{
public:
	std::vector<Var> args;
	Infer::LocalEnv* env;

	inline LambdaExp (
			const std::vector<Var>& _args,
			ExpPtr body)
		: Exp({ body }),
		  args(_args),
		  env(nullptr) {}
	virtual ~LambdaExp ();
};
class ObjectExp : public Exp
{
public:
	using Init = std::pair<std::string, ExpPtr>;

	TypePtr objType;
	std::vector<std::string> inits;

	Env::Type* base;
	std::vector<size_t> index;

	ObjectExp (
		TypePtr _objType,
		const std::vector<Init>& _inits);
	virtual ~ObjectExp ();
};
class CondExp : public Exp
{
public:
	inline CondExp (ExpPtr _cond, ExpPtr _then, ExpPtr _else)
		: Exp({ _cond, _then, _else }) {}
	inline CondExp (ExpPtr _cond, ExpPtr _then)
		: Exp({ _cond, _then }) {}
	virtual ~CondExp ();
};
class LetExp : public Exp
{
public:
	PatPtr pattern;

	inline LetExp (PatPtr _pat, ExpPtr _init)
		: Exp({ _init }), pattern(_pat) {}
	virtual ~LetExp ();
};
class LazyOpExp : public Exp
{
public:
	enum Kind { Or, And };

	Kind kind;
	inline LazyOpExp (ExpPtr _a, Kind _kind, ExpPtr _b)
		: Exp({ _a, _b }), kind(_kind) {}
	virtual ~LazyOpExp ();
};
class CompareExp : public Exp
{
public:
	enum Kind { Lt, LtEq, Eq, NotEq, Gr, GrEq };

	Kind kind;
	inline CompareExp (ExpPtr _a, Kind _kind, ExpPtr _b) // sugar
		: Exp({ _a, _b }), kind(_kind) {}
	inline CompareExp (ExpPtr _a, Kind _kind)
		: Exp({ _a }), kind(_kind) {}
	virtual ~CompareExp ();
};
class FieldExp : public Exp
{
public:
	std::string name;
	int index;
	Env::Function* method;

	inline FieldExp (ExpPtr _a, const std::string& _name)
		: Exp({ _a }), name(_name), index(-1), method(nullptr) {}
	virtual ~FieldExp ();
};
class MethodExp : public Exp
{
public:
	std::string name;
	Env::Function* method;

	inline MethodExp (const std::string& _name)
		: name(_name), method(nullptr) {}
	virtual ~MethodExp ();
};
class MemberExp : public Exp
{
public:
	inline MemberExp (ExpPtr _a, ExpPtr _mem)
		: Exp({ _a, _mem }) {}
	virtual ~MemberExp ();
};
class CallExp : public Exp
{
public:
	Env::Function* function;

	inline CallExp (ExpPtr _fn, const ExpList& _args)
		: Exp({ _fn }), function(nullptr)
	{
		children.insert(children.begin() + 1,
			_args.begin(), _args.end());
	}
	virtual ~CallExp ();
};
class TypeHintExp : public Exp
{
public:
	TypePtr type;

	inline TypeHintExp (ExpPtr e, TypePtr _type)
		: Exp({ e }), type(_type) {}
	virtual ~TypeHintExp ();
};
class BlockExp : public Exp
{
public:
	bool unitResult;
	ExpPtr last;
	inline BlockExp (const ExpList& stmts, bool _unit)
		: Exp(stmts), last(_unit ? nullptr : stmts.back()) {}
	virtual ~BlockExp ();
};
class MatchExp : public Exp
{
public:
	using Case = std::pair<PatPtr, ExpPtr>;

	std::vector<PatPtr> patterns;
	MatchExp (ExpPtr cond, const std::vector<Case>& cases);
	virtual ~MatchExp ();
};
class AssignExp : public Exp
{
public:
	inline AssignExp (ExpPtr _lh, ExpPtr _rh)
		: Exp({ _lh, _rh }) {}
	virtual ~AssignExp ();
};
class ReturnExp : public Exp
{
public:
	explicit inline ReturnExp (ExpPtr _what)
		: Exp({ _what }) {}
	inline ReturnExp () {}
	virtual ~ReturnExp ();
};
class WhileExp : public Exp
{
public:
	inline WhileExp (ExpPtr _cond, ExpPtr _body)
		: Exp({ _cond, _body }) {}
	virtual ~WhileExp ();
};

// utility to easily create
//  obj.method(args...)
ExpPtr methodCall (const Span& span,
	ExpPtr obj, const std::string& method,
	const ExpList& args);
ExpPtr methodCall (ExpPtr obj, const std::string& method,
	const ExpList& args);








class Pat
{
public:
	Span span;
	bool rootPosition;
	inline Pat ()
		: rootPosition(false) {}
	virtual ~Pat ();
	virtual bool canFail () const;

	static PatPtr cons (Span sp, PatPtr hd, PatPtr tl);
	static PatPtr nil (Span sp);
};
class ConstPat : public Pat
{
public:
	ExpPtr exp;
	Env::Function* equals;

	inline ConstPat (ExpPtr _exp)
		: exp(_exp) {}
	virtual ~ConstPat ();
};
class BindPat : public Pat
{
public:
	std::string name;
	Infer::LocalVar* var;

	inline BindPat (const std::string& _name)
		: name(_name), var(nullptr) {}
	virtual ~BindPat ();
	virtual bool canFail () const;
};
class EnumPat : public Pat
{
public:
	enum Kind {
		Enum, List, Tuple
	};
	Kind kind;
	std::vector<PatPtr> args;
	Name name;

	Env::Function* ctor;
	Infer::LocalVar* var;

	inline EnumPat (const Name& _name,
			const std::vector<PatPtr>& _args)
		: kind(Enum), args(_args), name(_name), ctor(nullptr), var(nullptr) {}
	inline EnumPat (Kind _kind, const std::vector<PatPtr>& _args)
		: kind(_kind), args(_args), name(""), ctor(nullptr), var(nullptr) {}
	virtual ~EnumPat ();
	virtual bool canFail () const;
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
	TypeList ifaces;
	
	inline ParamType (const std::string& _name,
			const TypeList& _ifaces)
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
class FuncType : public ConcreteType
{
public:
	inline FuncType (const TypeList& _args)
		: ConcreteType(Name("fn"), _args) {}
	~FuncType ();

	virtual std::string str () const;
};
class TupleType : public ConcreteType
{
public:
	inline TupleType (const TypeList& _args)
		: ConcreteType(Name("()"), _args) {}
	~TupleType ();

	virtual std::string str () const;
};



class Decl
{
public:
	virtual ~Decl () = 0;
	inline Decl ()
		: isPublic(false) {}

	Span span;
	bool isPublic;

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

	bool isExtern;
	std::string pkg;
	std::string key;
	TypePtr ret;

	ExpPtr body;

	Var impl;

	inline FuncDecl (const std::string& _name, 
			const std::vector<Var>& _args, 
			ExpPtr _body)
		: name(_name), args(_args),
		  isExtern(false), ret(nullptr), body(_body),
		  impl { "", nullptr } {}
	inline FuncDecl (const std::string& _name,
			const std::vector<Var>& _args, 
			const std::string& _pkg, const std::string& _key,
			TypePtr _ret)
		: name(_name), args(_args),
		  isExtern(true), pkg(_pkg), key(_key), ret(_ret),
		  impl { "", nullptr } {}

	virtual ~FuncDecl ();
};

class ConstDecl : public Decl
{
public:
	std::string name;
	TypePtr type;
	ExpPtr init;

	inline ConstDecl (const std::string& _name,
			TypePtr _type)
		: name(_name), type(_type), init(nullptr) {}
	inline ConstDecl (const std::string& _name,
			ExpPtr _init)
		: name(_name), type(nullptr), init(_init) {}
	virtual ~ConstDecl ();
};

struct EnumFunc
{
	std::string name;
	std::vector<TypePtr> args;
	Span span;
};

class TypeDecl : public Decl
{
public:
	std::string name;
	std::vector<std::string> args;

	bool isEnum;
	bool isExtern;
	bool gcCollect;

	std::vector<Var> fields;
	std::vector<EnumFunc> enumfns;

	inline TypeDecl (const std::string& _name,
			const std::vector<std::string>& _args,
			const std::vector<Var>& _fields)
		: name(_name), args(_args), 
		  isEnum(false), isExtern(false), fields(_fields) {}
	inline TypeDecl (const std::string& _name,
			const std::vector<std::string>& _args,
			bool _gc)
		: name(_name), args(_args),
		  isEnum(false), isExtern(true), gcCollect(_gc) {}
	inline TypeDecl (const std::string& _name,
			const std::vector<std::string>& _args,
			const std::vector<EnumFunc>& _enum)
		: name(_name), args(_args),
		  isEnum(true), isExtern(false), enumfns(_enum) {}
	virtual ~TypeDecl ();
};

struct IFaceFunc
{
	std::string name;
	std::vector<TypePtr> args;
	TypePtr ret;
	Span span;
};

class IFaceDecl : public Decl
{
public:
	std::string name;
	std::vector<std::string> args;

	std::string selfParam;
	std::vector<IFaceFunc> funcs;

	IFaceDecl (const std::string& _self, 
			const std::string& _name,
			const std::vector<std::string>& _args)
		: name(_name), args(_args), selfParam(_self) {}
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

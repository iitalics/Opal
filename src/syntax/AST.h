#pragma once
#include <syntax/Span.h>
#include <list.h>

namespace AST
{
class Exp;
class Type;
class Func;
class IFace;
class Constant;
using ExpPtr = std::shared_ptr<Exp>;
using ExpList = std::vector<ExpPtr>;
using TypePtr = std::shared_ptr<Type>;
using TypeList = list<TypePtr>;
using FuncPtr = std::shared_ptr<Func>;
using ConstantPtr = std::shared_ptr<Constant>;
using IFacePtr = std::shared_ptr<IFace>;

struct Name
{
	std::string name;
	std::string module;

	inline Name (const std::string& _name,
			const std::string& _module = "")
		: name(_name), module(_module) {}

	bool hasModule () const;
};

class Exp
{
public:
	explicit inline Exp (const ExpList& _children = {})
		: children(_children) {}
	virtual ~Exp () = 0;

	template <typename T>
	inline bool is () const { return dynamic_cast<T*>(this) != nullptr; }

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
};
class IntExp : public Exp
{
public:
	int value;
	explicit inline IntExp (int _value) : value(_value) {}
	virtual ~IntExp();
};
class RealExp : public Exp
{
public:
	float value;
	explicit inline RealExp (float _value) : value(_value) {}
	virtual ~RealExp ();
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
class TupleExp : public Exp
{
public:
	explicit inline TupleExp (const ExpList& ch) : Exp(ch) {}
	virtual ~TupleExp ();
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
};
class ObjectExp : public Exp
{
public:
	using Init = std::pair<std::string, ExpPtr>;

	TypePtr objType;
	std::vector<std::string> inits;

	inline ObjectExp (
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
	std::string name;
	TypePtr varType;
	int varID;

	inline LetExp (const std::string& _name, ExpPtr _init)
		: Exp({ _init }), name(_name), varType(nullptr) {}
	inline LetExp (const std::string& _name, TypePtr _type)
		: name(_name), varType(_type) {}
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
	virtual ~CompareExp ();
};
class ConsExp : public Exp
{
public:
	inline ConsExp (ExpPtr _hd, ExpPtr _tl)
		: Exp({ _hd, _tl }) {}
	virtual ~ConsExp ();
};
class NilExp : public Exp { public: virtual ~NilExp (); };
class FieldExp : public Exp
{
public:
	std::string name;
	inline FieldExp (ExpPtr _a, const std::string& _name)
		: Exp({ _a }), name(_name) {}
	virtual ~FieldExp ();
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
	inline CallExp (ExpPtr _fn, const ExpList& _args)
		: Exp({ _fn })
	{
		children.insert(children.begin() + 1,
			_args.begin(), _args.end());
	}
	virtual ~CallExp ();
};
class BlockExp : public Exp
{
public:
	bool unitResult;
	inline BlockExp (const ExpList& stmts, bool _unitResult)
		: Exp(stmts), unitResult(_unitResult) {}
	virtual ~BlockExp ();
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
};
class ReturnExp : public Exp
{
public:
	explicit inline ReturnExp (ExpPtr _what)
		: Exp({ _what }) {}
	inline ReturnExp () {}
	virtual ~ReturnExp ();
};
class GotoExp : public Exp
{
public:
	enum Kind { Break, Continue };
	Kind kind;
	explicit inline GotoExp (Kind _kind)
		: kind(_kind) {}
	virtual ~GotoExp ();
};



class Type
{
public:
	virtual ~Type () = 0;

	template <typename T>
	inline bool is () const { return dynamic_cast<T*>(this) != nullptr; }
};
class ParamType
{
public:
	std::string name;
	std::vector<Name> ifaces;
	inline ParamType (const std::string& _name,
			const std::vector<Name>& _ifaces)
		: name(_name), ifaces(_ifaces) {}
	virtual ~ParamType ();
};
class ConcreteType
{
public:
	Name name;
	TypeList subtypes;
	inline ConcreteType (const Name& _name,
			const TypeList& _subtypes)
		: name(_name), subtypes(_subtypes) {}
	virtual ~ConcreteType ();
};


struct FuncArg
{
	std::string name;
	TypePtr type;
};
using FuncArgs = std::vector<FuncArg>;

class Func
{
public:
	std::string name;
	FuncArgs args;
	ExpPtr body;

	FuncArg impl;

	inline Func (const std::string& _name,
			const FuncArgs& _args,
			ExpPtr _body)
		: name(_name), args(_args), body(_body),
		  impl { "", nullptr } {}
	~Func ();

	void setImpl (const std::string& name, TypePtr type);
};

class Constant
{
public:
	std::string name;
	TypePtr type;
	ExpPtr init;

	inline Constant (const std::string& _name, TypePtr _type)
		: name(_name), type(_type), init(nullptr) {}
	inline Constant (const std::string& _name, ExpPtr _init)
		: name(_name), type(nullptr), init(_init) {}
	~Constant ();
};

struct IFaceFunc
{
	std::string name;
	FuncArgs args;
	TypePtr ret;
};

class IFace
{
public:
	std::string name;
	TypePtr poly;
	std::vector<IFaceFunc> funcs;

	inline IFace (const std::string& _name, TypePtr _poly)
		: name(_name), poly(_poly) {}
	~IFace ();

	void add (const std::string& name, FuncArgs args, TypePtr ret);
};


class Toplevel
{
public:
	Toplevel ();
	~Toplevel ();

	std::string module;
	std::vector<std::string> uses;
	std::vector<FuncPtr> funcs;
	std::vector<IFacePtr> ifaces;
	std::vector<ConstantPtr> constants;
};


}
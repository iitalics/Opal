#pragma once
#include "Type.h"
#include "../env/Env.h"
#include <set>
namespace Opal { namespace Infer {
;

struct LocalVar
{
	LocalEnv* parent;
	std::string name;
	TypePtr type;
	bool didMut;
	bool didRef;

	inline bool needsBox () const
	{ return didMut && didRef; }
};
struct LocalEnv
{
	LocalEnv ();
	~LocalEnv ();
	LocalEnv* containing;

	// variables defined in this environment
	std::vector<LocalVar*> defs;

	// variables referenced by this environment
	std::vector<LocalVar*> refs;

	LocalVar* define (const std::string& name, TypePtr ty);
	void ref (LocalVar* var);
	size_t index (LocalVar* var) const;
	size_t size () const;
};

class Analysis
{
public:
	Analysis (Env::Function* parent, Analysis* calledBy);
	~Analysis ();

	static void initTypes ();

	using Depends = std::set<Analysis*>;

	TypePtr ret;
	LocalEnv* env;
	std::vector<LocalVar*> stack;
	Depends* depends;
	Env::Function* parent;

	// local variables
	LocalVar* get (const std::string& name) const;
	LocalVar* let (const std::string& name, TypePtr type);
	LocalVar* temp (TypePtr type = nullptr);
	// stack manip
	size_t stackSave ();
	void stackRestore (size_t n);

	// poly <-> param  type conversion utilities
	static TypePtr replaceParams (TypePtr ty, std::vector<TypePtr>& with);
	TypePtr polyToParam (TypePtr type);

	// type inference here
	void infer (AST::ExpPtr e, TypePtr dest);
	void infer (AST::PatPtr e, TypePtr dest);
	void unify (TypePtr dest, TypePtr src, const Span& span);

	// make this function depend on another one finishing
	void dependOn (Analysis* other);
	// mark this as finished
	void finish ();
	// am I and all my dependencies finished?
	bool allFinished () const;

private:
	enum {
		UnifyOK = 0,
		FailBadMatch,
		FailInfinite,
		FailSubscribe,
		FailMerge
	};

	Env::Namespace* nm;
	Type::Ctx _ctx;
	Analysis* _calledBy;
	bool _finished;
	size_t _temps;

	// failure information from _unify() and friends
	TypePtr _failType, _failIFace, _failIFace2;
	std::string _failName;

	TypePtr polyToParam (TypePtr type, 
			std::map<TypeWeakList*, TypePtr>& with);

	// unify types and check ifaces. somewhat
	//  based on Hindly-Milner
	int _unify (TypePtr dest, TypePtr src);
	bool _subscribes (TypePtr iface, TypePtr type);

	// merging ifaces from polytype lists
	struct Merge {
		std::vector<TypePtr> ifaces;
		std::map<std::string, TypePtr> methods;

		Merge ();
		bool addIFace (TypePtr obj, TypePtr iface);
		TypePtr finish ();
	};
	bool _merge (TypePtr a, TypePtr b);

	// get function type and do circular inference
	//  shenanigans if required
	TypePtr _getFuncType (Env::Function* func);

	// find fields and methods from a type
	TypePtr _findProperty (TypePtr obj,
		const std::string& name, int& out);
	TypePtr _findMethod (TypePtr obj,
		const std::string& name, Env::Function*& out);
	TypePtr _findIFaceFunc (TypePtr obj, TypePtr iface,
		const std::string& name, Env::Function*& out);
	// replace params from 'type' using 'obj' and 'self'
	static TypePtr _inst (TypePtr obj, TypePtr type, TypePtr self = nullptr);
	TypePtr _instMethod (TypePtr obj, Env::Function* fn);

	// OOP is for losers
	void _infer (AST::VarExp* e, TypePtr dest);
	void _infer (AST::NumberExp* e, TypePtr dest);
	void _infer (AST::FieldExp* e, TypePtr dest);
	void _infer (AST::PropertyExp* e, TypePtr dest);
	void _infer (AST::CallExp* e, TypePtr dest);
	void _infer (AST::TypeHintExp* e, TypePtr dest);
	void _infer (AST::BlockExp* e, TypePtr dest);
	void _infer (AST::TupleExp* e, TypePtr dest);
	void _infer (AST::CondExp* e, TypePtr dest);
	void _infer (AST::LazyOpExp* e, TypePtr dest);
	void _infer (AST::CompareExp* e, TypePtr dest);
	void _infer (AST::ObjectExp* e, TypePtr dest);
	void _infer (AST::LambdaExp* e, TypePtr dest);
	void _infer (AST::MethodExp* e, TypePtr dest);
	void _infer (AST::MatchExp* e, TypePtr dest);
	void _infer (AST::ReturnExp* e);
	void _infer (AST::LetExp* e);
	void _infer (AST::AssignExp* e);
	void _infer (AST::WhileExp* e);

	void _infer (AST::ConstPat* p, TypePtr dest);
	void _infer (AST::BindPat* p, TypePtr dest);
	void _infer (AST::EnumPat* p, TypePtr dest);
	void _inferTuplePat (AST::EnumPat* p, TypePtr dest);
};


}}

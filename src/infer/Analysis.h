#pragma once
#include "Type.h"
#include "../env/Env.h"
#include <set>
namespace Opal { namespace Infer {
;


class Analysis
{
public:
	Analysis (Env::Function* parent, Analysis* calledBy);
	~Analysis ();

	struct LocalVar
	{
		int id;
		std::string name;
		TypePtr type;
		bool mut;
	};
	using Depends = std::set<Analysis*>;

	TypePtr ret;
	std::vector<LocalVar> allVars;
	std::vector<int> stack;
	Depends* depends;
	Env::Function* parent;

	// local variable lookup
	int get (const std::string& name) const;
	int let (const std::string& name, TypePtr type);

	// poly <-> param  type conversion utilities
	TypePtr replaceParams (TypePtr ty, std::vector<TypePtr>& with);
	TypePtr polyToParam (TypePtr type);

	// type inference here
	void infer (AST::ExpPtr e, TypePtr dest);
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

	// failure information from _unify() and friends
	TypePtr _failType, _failIFace, _failIFace2;
	std::string _failName;

	TypePtr polyToParam (TypePtr type, 
			std::map<TypeWeakList*, TypePtr>& with);

	// unify types and check ifaces. somewhat
	//  based on Hindly-Milner
	int _unify (TypePtr dest, TypePtr src);
	bool _subscribes (TypePtr iface, TypePtr type);
	bool _mergePoly (TypePtr a, TypePtr b);
	bool _mergePolyAdd (TypeList& list, TypePtr iface);

	// get function type and do circular inference
	//  shenanigans if required
	TypePtr _getFuncType (Env::Function* func);

	// find fields and methods from a type
	TypePtr _findField (TypePtr obj,
		const std::string& name, int& out);
	TypePtr _findMethod (TypePtr obj,
		const std::string& name, Env::Function*& out);
	TypePtr _findIFaceFunc (TypePtr obj, TypePtr iface,
		const std::string& name, Env::Function*& out);
	// replace params from 'type' using 'obj' and 'self'
	TypePtr _inst (TypePtr obj, TypePtr type,
					TypePtr self = nullptr);
	TypePtr _instMethod (TypePtr obj, Env::Function* fn);

	// OOP is for losers
	void _infer (AST::VarExp* e, TypePtr dest);
	void _infer (AST::IntExp* e, TypePtr dest);
	void _infer (AST::FieldExp* e, TypePtr dest);
	void _infer (AST::CallExp* e, TypePtr dest);
	void _infer (AST::BlockExp* e, TypePtr dest);
	void _infer (AST::TupleExp* e, TypePtr dest);
	void _infer (AST::CondExp* e, TypePtr dest);
	void _infer (AST::LazyOpExp* e, TypePtr dest);
	void _infer (AST::CompareExp* e, TypePtr dest);
	void _infer (AST::ObjectExp* e, TypePtr dest);
	void _infer (AST::ReturnExp* e);
	// void _infer (AST::LetExp* e);
	// void _infer (AST::AssignExp* e);
	// void _infer (AST::WhileExp* e);
};


}}

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

	int get (const std::string& name) const;
	int let (const std::string& name, TypePtr type);

	TypePtr replaceParams (TypePtr ty, std::vector<TypePtr>& with);
	TypePtr polyToParam (TypePtr type);

	void infer (AST::ExpPtr e, TypePtr dest);
	void unify (TypePtr dest, TypePtr src, const Span& span);

	void dependOn (Analysis* other);
	void finish ();
	bool allFinished () const;

private:
	enum {
		UnifyOK = 0,
		FailBadMatch,
		FailInfinite
	};

	Env::Namespace* nm;
	Type::Ctx _ctx;
	Analysis* _calledBy;
	bool _finished;

	TypePtr polyToParam (TypePtr type, 
			std::map<TypeWeakList*, TypePtr>& with);

	int _unify (TypePtr dest, TypePtr src);
	TypePtr _getFuncType (Env::Function* func);

	TypePtr _findField (TypePtr obj,
		const std::string& name, int& out);
	TypePtr _findMethod (TypePtr obj,
		const std::string& name, Env::Function*& out);
	TypePtr _findIFaceFunc (TypePtr obj, TypePtr iface,
		const std::string& name, Env::Function*& out);
	TypePtr _inst (TypePtr obj, TypePtr type);
	TypePtr _instMethod (TypePtr obj, Env::Function* fn);

	void _infer (AST::VarExp* e, TypePtr dest);
	void _infer (AST::IntExp* e, TypePtr dest);
	void _infer (AST::FieldExp* e, TypePtr dest);
	void _infer (AST::CallExp* e, TypePtr dest);
	void _infer (AST::BlockExp* e, TypePtr dest);
	void _infer (AST::TupleExp* e, TypePtr dest);
	void _infer (AST::CondExp* e, TypePtr dest);
	void _infer (AST::LazyOpExp* e, TypePtr dest);
	void _infer (AST::CompareExp* e, TypePtr dest);
	// void _infer (AST::ObjectExp* e, TypePtr dest);
	// void _infer (AST::ReturnExp* e);
	// void _infer (AST::LetExp* e);
	// void _infer (AST::AssignExp* e);
	// void _infer (AST::WhileExp* e);
};


}}

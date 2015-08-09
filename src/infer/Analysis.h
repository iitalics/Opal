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
	TypePtr _getFieldType (int& idx, Env::Type* base, const std::string& name);
	TypePtr _getIFaceFuncType (Env::Type* base, const std::string& name);
	TypePtr _getMethodType (Env::Function*& fnout, Env::Type* base, const std::string& name);
	TypePtr _instField (TypePtr self, TypePtr type);
	TypePtr _instIFace (TypePtr self, TypePtr type);
	TypePtr _instMethod (TypePtr self, TypePtr type, Env::Function* fn);

	void _infer (AST::VarExp* e, TypePtr dest);
	void _infer (AST::IntExp* e, TypePtr dest);
	void _infer (AST::FieldExp* e, TypePtr dest);
	void _infer (AST::CallExp* e, TypePtr dest);
	void _infer (AST::BlockExp* e, TypePtr dest);
	void _infer (AST::TupleExp* e, TypePtr dest);
	void _infer (AST::CondExp* e, TypePtr dest);
};


}}

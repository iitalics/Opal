#pragma once
#include "Type.h"
#include "../env/Env.h"
namespace Opal { namespace Infer {
;


class Analysis
{
public:
	Analysis (Env::Namespace* _nm,
			const std::vector<Var>& args);
	~Analysis ();

	struct LocalVar
	{
		int id;
		std::string name;
		TypePtr type;
		bool mut;
	};

	std::vector<LocalVar> allVars;
	std::vector<int> stack;

	int get (const std::string& name) const;
	int let (const std::string& name, TypePtr type);

	TypePtr newType (const TypeList& ifaces = TypeList());
	TypePtr replaceParams (TypePtr ty, std::vector<TypePtr>& with);

	void infer (AST::ExpPtr e, TypePtr dest);

	void unify (TypePtr dest, TypePtr src, const Span& span);

	void polyToParam (TypePtr type);

private:
	Env::Namespace* nm;
	Type::Ctx _ctx;

	enum {
		UnifyOK = 0,
		FailBadMatch,
		FailInfinite
	};

	int _unify (TypePtr dest, TypePtr src);

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

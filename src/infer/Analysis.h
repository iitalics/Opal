#pragma once
#include "Type.h"
#include "../env/Env.h"
namespace Opal { namespace Infer {
;


class Analysis
{
public:
	Analysis (Env::Namespace* _nm);
	~Analysis ();

	struct LocalVar
	{
		int id;
		std::string name;
		TypePtr type;
		bool mut;
	};

	Type::Ctx ctx;
	std::vector<LocalVar> allVars;
	std::vector<int> stack;

	int get (const std::string& name) const;
	int let (const std::string& name, TypePtr type);

	TypePtr newType ();

	void infer (AST::ExpPtr e, TypePtr ctx);

	void unify (TypePtr dest, TypePtr src, const Span& span);
	void set (int polyId, TypePtr res);

private:
	Env::Namespace* nm;
	std::vector<TypePtr> _polies;
	int _polyCount;

	void _infer (AST::VarExp* e, TypePtr ctx);
	void _infer (AST::IntExp* e, TypePtr ctx);
	void _infer (AST::FieldExp* e, TypePtr ctx);
	void _infer (AST::CompareExp* e, TypePtr ctx);
	void _infer (AST::BlockExp* e, TypePtr ctx);
};


}}

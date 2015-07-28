#include "syntax/Scanner.h"
#include "syntax/Parse.h"
using namespace Opal;


static void show_fn (
	const std::string& name,
	const std::vector<AST::Var>& args,
	bool newline = true)
{
	std::cout << "  fn " << name << "(";
	bool first = true;
	for (auto& v : args)
	{
		if (first)
			first = false;
		else
			std::cout << ", ";
		std::cout << v.name << " : " << v.type->str();
	}
	std::cout << ")";
	if (newline)
		std::cout << std::endl;
}
int main ()
{
	SourceError::color = true;

	try
	{
		Scanner scan("tests/syntax-toplevel.opal");

		auto top = Parse::parseToplevel(scan);

		for (auto decl : top.decls)
		{
			auto fn = dynamic_cast<AST::FuncDecl*>(decl.get());
			auto iface = dynamic_cast<AST::IFaceDecl*>(decl.get());
			if (fn != nullptr)
			{
				if (fn->impl.type != nullptr)
					std::cout << "  impl  " << fn->impl.name << " : "
				              << fn->impl.type->str() << std::endl;

				show_fn(fn->name, fn->args, true);
				std::cout << "  " << fn->body->str(2) << std::endl;
				std::cout << std::endl;
			}
			else if (iface != nullptr)
			{
				std::cout << "  iface " << iface->name;
				if (!iface->selfParam.empty())
					std::cout << " #" << iface->selfParam << std::endl;
				else
					std::cout << std::endl;

				for (auto& fn : iface->funcs)
				{
					std::cout << "  ";
					show_fn(fn.name, fn.args, false);
					std::cout << " : " << fn.ret->str() << std::endl;
				}
				std::cout << std::endl;
			}
		}
	}
	catch (SourceError& err)
	{
		std::cerr << err.what() << std::endl;
	}
}
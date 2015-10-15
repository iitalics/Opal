#include <iostream>
#include "env/Loader.h"
#include "syntax/Parse.h"
#include "syntax/Desugar.h"
#include "code/CodeGen.h"
#include "runtime/Exec.h"
using namespace Opal;

static Env::Function* replGenerate (const std::string& string,
		Env::Namespace* nm, Env::Module* module,
		Run::ThreadPtr thread)
{
	// parse
	Scanner scan(string, "<input>");
	if (scan.get() == Tokens::END_OF_FILE)
		return nullptr;

	auto ast = Parse::parseExp(scan);
	scan.expect(Tokens::END_OF_FILE);
	Desugar::desugar(ast);

	// create
	auto fn = new Env::Function(Env::Function::CodeFunction,
		"<input>", module);
	fn->nm = nm;
	fn->body = ast;

	// analyze
	fn->infer();
	fn->code = Code::CodeGen::generate(fn);

	return fn;
}

void runRepl (Run::ThreadPtr thread)
{
	if (SourceError::color)
		std::cout << "\x1b[1m";
	std::cout << "@@@  Opal interactive prompt  @@@" << std::endl;
	if (SourceError::color)
		std::cout << "\x1b[0m";
	std::cout << "  enter !types to display value types" << std::endl
	          << std::endl;

	// create a namespace and private module
	auto module = new Env::Module("(input)");
	auto nm = new Env::Namespace(module, module);
	Env::Function* fn;
	bool showTypes = false;

	while (!std::cin.eof())
	{
		// Read
		std::cout << ">> ";
		std::cout.flush();

		std::string input;
		std::getline(std::cin, input);

		if (input == "!types")
		{
			showTypes = !showTypes;
			continue;
		}

		try
		{
			// Eval
			fn = replGenerate(input, nm, module, thread);
			if (fn == nullptr) continue;

			thread->call(fn);
			while (thread->step()) ;
		}
		catch (std::exception& err)
		{
			std::cerr << err.what() << std::endl;
			continue;
		}

		// Print
		auto res = thread->pop();
		std::cout << res.str(SourceError::color);
		if (showTypes)
			std::cout << " : " << fn->ret->str();
		std::cout << std::endl;

		res.release();
		delete fn;

		// Loop
	}
	std::cout << std::endl;
}

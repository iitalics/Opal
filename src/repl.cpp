#include <iostream>
#include "env/Loader.h"
#include "syntax/Parse.h"
#include "syntax/Desugar.h"
#include "code/CodeGen.h"
#include "runtime/Exec.h"
using namespace Opal;

static void replEvaluate (const std::string& string,
		Env::Namespace* nm, Env::Module* module,
		Run::ThreadPtr thread)
{
	// parse
	Scanner scan(string, "<input>");
	if (scan.get() == Tokens::END_OF_FILE)
		return;

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

	// execute
	thread->call(fn);
	while (thread->step()) ;

	delete fn;
}

void runRepl (Run::ThreadPtr thread)
{
	std::cout << "@@@  Opal interactive prompt  @@@"
	          << std::endl << std::endl;

	// create a namespace and private module
	auto module = new Env::Module();
	auto nm = new Env::Namespace(module, module);

	while (!std::cin.eof())
	{
		// Read
		std::cout << ">> ";
		std::cout.flush();

		std::string input;
		std::getline(std::cin, input);

		try
		{
			// Eval
			replEvaluate(input, nm, module, thread);
		}
		catch (std::exception& err)
		{
			std::cerr << err.what() << std::endl;
			continue;
		}

		// Print
		auto res = thread->pop();
		std::cout << res.str() << std::endl;
		res.release();

		// Loop
	}
	std::cout << std::endl;
}

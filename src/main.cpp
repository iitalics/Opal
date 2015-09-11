#include <iostream>
#include "env/Loader.h"
#include "infer/Analysis.h"
#include "runtime/Cell.h"
#include "runtime/Exec.h"
using namespace Opal;

int main (int argc, char** argv)
{
	if (argc < 1) return 2;

	SourceError::color = true;

	try
	{
		// look for modules in the current directory
		Env::initSearchPaths(argv[0]);

		// load some code
		auto nm = Env::loadSource("tests/patterns.opal");
		Env::finishModuleLoad();

		// find 'main' function
		auto global_main = nm->getGlobal(AST::Name("main"));
		if (global_main == nullptr || !global_main->isFunc)
			throw SourceError("no main function defined");

		// execute it in a new thread
		auto thread = Run::Thread::start();
		thread->call(global_main->func);
		std::cout << "executing main" << std::endl;

		// run threads until completion
		size_t count = 0;
		while (Run::Thread::stepAny()) count++;

		// log output
		std::cout << "total commands: " << count << std::endl;
		if (thread->size() > 0)
		{
			auto res = thread->pop();
			std::cout << "result: " << res.str() << std::endl;
			res.release();
		}
		else
			std::cout << "empty stack!" << std::endl;

		// bye
		// TODO: destroy everything?
	}
	catch (std::exception& err)
	{
		std::cerr << err.what() << std::endl;
		return 1;
	}
}
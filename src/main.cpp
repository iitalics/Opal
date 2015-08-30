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

	Run::Thread thread;
	try
	{
		// initialize
		// TODO: group this
		Env::initSearchPaths(argv[0]);
		Infer::Analysis::initTypes();
		Run::Cell::initTypes();

		// load some code
		auto nm = Env::loadSource("tests/external.opal");
		Env::finishModuleLoad();

		// find 'main' function
		auto global_main = nm->getGlobal(AST::Name("main"));
		if (global_main == nullptr)
			throw SourceError("no main function defined");

		// execute it in a new thread
		std::cout << "executing main" << std::endl;
		size_t count = 0;
		thread.call(global_main->func);
		while (thread.step()) count++;

		// log output
		std::cout << "total commands: " << count << std::endl;
		if (thread.size() > 0)
		{
			auto res = thread.pop();
			std::cout << "result: " << res.str() << std::endl;
			res.release();
		}
		else
			std::cout << "empty stack!" << std::endl;

		// bye
		// TODO: destroy everything?
	}
	catch (SourceError& err)
	{
		std::cerr << err.what() << std::endl;
		return 1;
	}
}
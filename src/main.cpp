#include "env/Loader.h"
#include "infer/Analysis.h"
#include "runtime/Cell.h"
#include "runtime/Exec.h"
using namespace Opal;


static void foo (Run::Thread& th)
{
	std::cout << "===> foo! <===" << std::endl;
	th.push(Run::Cell::Unit());
}
static void loadPackage (Env::Package& pkg)
{
	pkg.put("foo", foo);

	std::cout << "load package '" << pkg.name() << "' is a success" << std::endl;
}
static Env::PackageLoad _1("opal.test", loadPackage);



int main ()
{
	SourceError::color = true;

	Run::Thread thread;
	try
	{
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
		thread.call(global_main->func);

		size_t count = 0;
		while (thread.step()) count++;

		std::cout << "total commands: " << count << std::endl;
		if (thread.size() > 0)
		{
			auto res = thread.pop();
			std::cout << "result: " << res.str() << std::endl;
			res.release();
		}

		// bye
	}
	catch (SourceError& err)
	{
		std::cerr << err.what() << std::endl;
	}
}
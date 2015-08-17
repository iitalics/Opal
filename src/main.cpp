#include "env/Loader.h"
#include "infer/Analysis.h"
#include "runtime/Cell.h"
#include "runtime/Exec.h"
using namespace Opal;


int main ()
{
	SourceError::color = true;

	Run::Thread thread;
	try
	{
		Env::Module::getCore();
		Infer::Analysis::initTypes();
		Run::Cell::initTypes();

		auto nm = Env::loadSource("tests/expressions.opal");
		Env::finishModuleLoad();

		// find 'main' function
		auto g_main = nm->getGlobal(AST::Name("main"));
		if (g_main == nullptr)
			throw SourceError("no main function defined");
		auto fn_main = g_main->func;
		auto code = fn_main->code;

		// execute it in a new thread
		std::cout << "executing main" << std::endl;
		thread.call(code);
		while (thread.step()) ;

		// bye
	}
	catch (SourceError& err)
	{
		std::cerr << err.what() << std::endl;
	}
}
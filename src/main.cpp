#include "env/Loader.h"
#include "infer/Analysis.h"
#include "runtime/Cell.h"
#include "runtime/Exec.h"
using namespace Opal;



Run::Code example_program ()
{
	using namespace Opal::Run;

	auto prgm = new Cmd[64];

	prgm[0] = Cmd { Cmd::Load, .var = 0 };
	prgm[1] = Cmd { Cmd::Store, .var = 1 };
	prgm[2] = Cmd { Cmd::Jump, .dest_pc = 0 };
	prgm[3] = Cmd { Cmd::Ret };

	return Run::Code(prgm, 0, 1);
}

int main ()
{
	SourceError::color = true;

	try
	{
		Env::Module::getCore();
		Infer::Analysis::initTypes();
		Run::Cell::initTypes();

		Run::Thread thread;
		auto code = example_program();

		thread.call(code);
		while (thread.step()) ;

	//	auto nm = Env::loadSource("tests/expressions.opal");
		Env::finishModuleLoad();
	}
	catch (SourceError& err)
	{
		std::cerr << err.what() << std::endl;
	}
}
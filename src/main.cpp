#include "env/Loader.h"
#include "infer/Analysis.h"
#include "runtime/Cell.h"
using namespace Opal;


int main ()
{
	SourceError::color = true;

	try
	{
		Env::Module::getCore();
		Infer::Analysis::initTypes();
		Run::Cell::initTypes();

		auto nm = Env::loadSource("tests/expressions.opal");
		Env::finishModuleLoad();
	}
	catch (SourceError& err)
	{
		std::cerr << err.what() << std::endl;
	}
}